#include <shared.h>

#include <algorithm>
#include <cassert>
#include <cstring>

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

unsigned char *serialize_packet(Packet packet)
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
