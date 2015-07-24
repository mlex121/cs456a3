#include <sender.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <shared.h>

namespace a3 {

const char *CHANNEL_INFO_FILENAME = "channelInfo";

Sender::Sender(uint32_t timeout, const std::string &filename) :
    m_timeout(timeout),
    m_filename(filename),
    m_sock_fd(-1),
    m_dest_hostname(nullptr),
    m_dest_port(nullptr)
{
    if (read_addrinfo() != 0) {
        std::cerr << "Error reading address info from " << CHANNEL_INFO_FILENAME << '\n';
        ::exit(EXIT_FAILURE);
    }

    if (setup_socket() != 0) {
        std::cerr << "Error creating socket" << '\n';
        ::exit(EXIT_FAILURE);
    }
}

Sender::~Sender()
{
    if (m_sock_fd != -1) {
        ::close(m_sock_fd);
        m_sock_fd = -1;
    }

    if (m_dest_hostname != nullptr) {
        delete[] m_dest_hostname;
        m_dest_hostname = nullptr;
    }

    if (m_dest_port != nullptr) {
        delete[] m_dest_port;
        m_dest_port = nullptr;
    }
}

int Sender::read_addrinfo()
{
    std::ifstream file(CHANNEL_INFO_FILENAME);
    std::string dest_hostname;
    std::string dest_port;

    if (!(file >> dest_hostname >> dest_port)) {
        return -1;
    }

    m_dest_hostname = new char[dest_hostname.size()];

    if (m_dest_hostname == nullptr) {
        return -2;
    }

    std::strncpy(m_dest_hostname, dest_hostname.c_str(), dest_hostname.size());

    m_dest_port = new char[dest_port.size()];

    if (m_dest_port == nullptr) {
        delete[] m_dest_hostname;
        m_dest_hostname = nullptr;
        return -3;
    }

    std::strncpy(m_dest_port, dest_port.c_str(), dest_port.size());
    return 0;
}

int Sender::setup_socket()
{
    int ret = -1;
    struct addrinfo *recvinfo = nullptr;
    struct addrinfo hints = {};

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    ret = ::getaddrinfo(m_dest_hostname, m_dest_port, &hints, &recvinfo);

    if (ret != 0) {
        std::cerr << "getaddrinfo: " << ::gai_strerror(ret) << '\n';
        ::freeaddrinfo(recvinfo);
        return ret;
    }

    struct addrinfo *p = nullptr;
    int sock_fd = -1;

    for (p = recvinfo; p != nullptr; p = p->ai_next) {
        sock_fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (sock_fd == -1) {
            continue;
        }

        if (::connect(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
            ::close(sock_fd);
            continue;
        }

        // If we've successfully connected, don't need to keep iterating through
        break;
    }

    if (p == nullptr) {
        std::cerr << "Could not connect to any addrinfo from getaddrinfo()" << '\n';
        ::freeaddrinfo(recvinfo);
        return -1;
    }

    ::freeaddrinfo(recvinfo);
    return 0;
}

int Sender::send_packet(Packet packet)
{
    unsigned char *serialized = serialize_packet(packet);

    if (serialized == nullptr) {
        return -1;
    }

    size_t size = packet.length;
    size_t offset = 0;

    while (offset < size) {
        int sent = ::send(m_sock_fd, &serialized[0] + offset, size - offset, 0);

        if (sent == -1) {
            std::perror("send");
            delete[] serialized;
            return -1;
        }

        offset += sent;
    }

    assert(offset == size);
    delete[] serialized;
    return 0;
}

Packet Sender::receive_reply() {
    int size = 0;
    size_t offset = 0;
    unsigned char header[PACKET_HEADER_SIZE];

    // Loop until we retrieve the full header
    do {
        size = ::recv(m_sock_fd, &header[0] + offset, PACKET_HEADER_SIZE - offset, 0);
    } while (offset < PACKET_HEADER_SIZE && size > 0);

    if (offset != PACKET_HEADER_SIZE) {
        return create_invalid_packet();
    }

    uint32_t packet_length = get_length_from_packet_header(header);

    if (packet_length < PACKET_HEADER_SIZE || MAX_PACKET_SIZE < packet_length) {
        return create_invalid_packet();
    }

    unsigned char *serialized = new unsigned char[packet_length];

    if (serialized == nullptr) {
        return create_invalid_packet();
    }

    // Copy the header over to our new buffer
    std::memcpy(&serialized[0], &header[0], PACKET_HEADER_SIZE);

    // Loop until we get the whole packet
    do {
        size = ::recv(m_sock_fd, &serialized[0] + offset, packet_length - offset, 0);
    } while (offset < packet_length && size > 0);

    if (offset != packet_length) {
        return create_invalid_packet();
    }

    Packet packet = deserialize_packet(&serialized[0]);
    delete[] serialized;
    return packet;
}

} // namespace a3
