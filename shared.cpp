#include <shared.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>

#include <arpa/inet.h>

namespace a3 {

static const size_t PACKET_TYPE_OFFSET = 0;
static const size_t PACKET_SEQ_NUM_OFFSET = PACKET_TYPE_OFFSET + sizeof(PacketType);
static const size_t PACKET_LENGTH_OFFSET = PACKET_SEQ_NUM_OFFSET + sizeof(uint32_t);
static const size_t PACKET_PAYLOAD_OFFSET = PACKET_HEADER_SIZE;

Packet create_packet(PacketType type, uint32_t seq_num, const unsigned char *payload, size_t payload_size)
{
    Packet packet = {};

    packet.type = type;
    packet.seq_num = seq_num;

    if (type == ACK || type == EOT || payload == nullptr) {
        packet.payload_size = 0;
    } else {
        packet.payload_size = std::min(MAX_PAYLOAD_SIZE, payload_size);
    }

    if (payload != nullptr) {
        std::memcpy(packet.payload, payload, packet.payload_size);
    }

    packet.length = PACKET_HEADER_SIZE + payload_size;

    return packet;
}

Packet create_ack(uint32_t seq_num)
{
    return create_packet(ACK, seq_num, nullptr, 0);
}

Packet create_eot()
{
    return create_packet(EOT, 0, nullptr, 0);
}

Packet create_invalid_packet()
{
    return create_packet(INVALID_PACKET, MAX_SEQ_NUM + 1, nullptr, 0);
}

bool equal_packets(const Packet &p1, const Packet &p2)
{
    if (p1.type != p2.type) {
        return false;
    }

    if (p1.seq_num != p2.seq_num) {
        return false;
    }

    if (p1.length != p2.length) {
        return false;
    }

    if (p1.payload_size != p2.payload_size) {
        return false;
    }

    if (std::memcmp(&p1.payload[0], &p2.payload[0], p1.payload_size) != 0) {
        return false;
    }

    return true;
}

unsigned char *serialize_packet(const Packet &packet)
{
    if (packet.length < PACKET_HEADER_SIZE) {
        return nullptr;
    }

    unsigned char *serialized = new unsigned char[packet.length];
    std::memset(serialized, 0, packet.length * sizeof(serialized[0]));

    if (serialized == nullptr) {
        return nullptr;
    }

    size_t offset = 0;
    uint32_t net_endian_long = 0;

    // Convert packet type to big endian and copy
    net_endian_long = htonl(packet.type);
    std::memcpy(serialized + offset, &net_endian_long, sizeof(packet.type));
    offset += sizeof(packet.type);

    // Sequence number
    net_endian_long = htonl(packet.seq_num);
    std::memcpy(serialized + offset, &net_endian_long, sizeof(packet.seq_num));
    offset += sizeof(packet.seq_num);

    // Length
    net_endian_long = htonl(packet.length);
    std::memcpy(serialized + offset, &net_endian_long, sizeof(packet.length));
    offset += sizeof(packet.length);

    // Payload
    std::memcpy(serialized + offset, packet.payload, packet.payload_size);
    offset += packet.payload_size;

    assert(offset == packet.length);

    return serialized;
}

Packet deserialize_packet(const unsigned char *serialized)
{
    if (serialized == nullptr) {
        return create_invalid_packet();
    }

    Packet packet = {};

    size_t offset = 0;
    uint32_t host_endian_long = 0;

    // Convert packet type from network endianness to host endianness
    std::memcpy(&host_endian_long, serialized + offset, sizeof(packet.type));
    packet.type = (PacketType)ntohl(host_endian_long);
    offset += sizeof(packet.type);

    // Sequence number
    assert(offset == PACKET_SEQ_NUM_OFFSET);
    std::memcpy(&host_endian_long, serialized + offset, sizeof(packet.seq_num));
    packet.seq_num = ntohl(host_endian_long);
    offset += sizeof(packet.seq_num);

    // Length
    assert(offset == PACKET_LENGTH_OFFSET);
    std::memcpy(&host_endian_long, serialized + offset, sizeof(packet.length));
    packet.length = ntohl(host_endian_long);
    offset += sizeof(packet.length);

    // Make sure we didn't mess up
    assert(packet.length >= PACKET_HEADER_SIZE);
    assert(packet.length <= MAX_PACKET_SIZE);

    packet.payload_size = packet.length - PACKET_HEADER_SIZE;

    // Payload
    assert(offset == PACKET_PAYLOAD_OFFSET);
    std::memcpy(packet.payload, serialized + offset, packet.payload_size);
    offset += packet.payload_size;

    assert(offset == packet.length);
    return packet;
}

int send_packet(int sock_fd, const Packet &packet, const struct sockaddr *to, socklen_t to_len)
{
    unsigned char *serialized = serialize_packet(packet);

    if (serialized == nullptr) {
        std::cerr << "Couldn't allocate buffer for packet serialization\n";
        return -1;
    }

    size_t size = packet.length;
    size_t offset = 0;

    while (offset < size) {
        int sent = ::sendto(
            sock_fd,
            &serialized[0] + offset,
            size - offset,
            0,
            to,
            to_len
        );

        if (sent == -1) {
            std::perror("sendto");
            delete[] serialized;
            return -2;
        }

        offset += sent;
    }

    assert(offset == size);
    std::cout << packet_to_log_string(packet, SEND) << '\n';
    delete[] serialized;
    return 0;
}

int receive_packet(int sock_fd, Packet &packet, struct sockaddr *from, socklen_t *from_len)
{
    int size = 0;
    size_t offset = 0;
    unsigned char header[PACKET_HEADER_SIZE];

    // Loop until we retrieve the full header
    do {
        size = ::recvfrom(
            sock_fd,
            &header[0] + offset,
            PACKET_HEADER_SIZE - offset,
            0,
            from,
            from_len
        );
        offset += size;
    } while (offset < PACKET_HEADER_SIZE && size > 0);

    if (size == -1) {
        std::perror("recvfrom");
        return -1;
    }

    if (offset != PACKET_HEADER_SIZE) {
        std::cerr << "Got packet header of the wrong size: " << offset << " (should be " << PACKET_HEADER_SIZE << ")\n";
        return -2;
    }

    uint32_t packet_length = get_length_from_packet_header(header);

    if (packet_length < PACKET_HEADER_SIZE) {
        std::cerr << "Received packet has incorrect length: " << packet_length << " (can't be less than " << PACKET_HEADER_SIZE << ")\n";
        return -3;
    }

    if (packet_length > MAX_PACKET_SIZE) {
        std::cerr << "Received packet has incorrect length: " << packet_length << " (can't be greater than " << MAX_PACKET_SIZE << ")\n";
        return -4;
    }

    unsigned char *serialized = new unsigned char[packet_length];

    if (serialized == nullptr) {
        std::cerr << "Couldn't allocate buffer for serialized packet\n";
        return -5;
    }

    // Copy the header over to our new buffer
    std::memcpy(&serialized[0], &header[0], PACKET_HEADER_SIZE);

    // Loop until we get the whole packet
    do {
        size = ::recvfrom(
            sock_fd,
            &serialized[0] + offset,
            packet_length - offset,
            0,
            from,
            from_len
        );
        offset += size;
    } while (offset < packet_length && size > 0);

    if (size == -1) {
        std::perror("recvfrom");
        delete[] serialized;
        return -6;
    }

    if (offset != packet_length) {
        std::cerr << "Didn't get entire packet (received " << offset << " bytes, expected " << packet_length << " bytes)\n";
        delete[] serialized;
        return -7;
    }

    packet = deserialize_packet(&serialized[0]);
    std::cout << packet_to_log_string(packet, RECV) << '\n';
    delete[] serialized;
    return 0;
}

uint32_t get_length_from_packet_header(const unsigned char header[PACKET_HEADER_SIZE])
{
    uint32_t length = 0;
    std::memcpy(&length, &header[0] + PACKET_LENGTH_OFFSET, sizeof(uint32_t));

    // Still need to convert endianness
    length = ntohl(length);

    assert(length >= PACKET_HEADER_SIZE);
    assert(length <= MAX_PACKET_SIZE);
    return length;
}

std::string packet_to_log_string(Packet packet, TransferDirection direction)
{
    std::string s = "PKT ";

    switch (direction) {
        case SEND:
            s.append("SEND ");
            break;
        case RECV:
            s.append("RECV ");
            break;
    }

    switch (packet.type) {
        case DAT:
            s.append("DAT ");
            break;
        case ACK:
            s.append("ACK ");
            break;
        case EOT:
            s.append("EOT ");
            break;
        case INVALID_PACKET:
            s.append("**INVALID** ");
            break;
    }

    // "<sequence number> <total length>"
    s.append(std::to_string(packet.seq_num));
    s.push_back(' ');
    s.append(std::to_string(packet.length));

    return s;
}

} // namespace a3
