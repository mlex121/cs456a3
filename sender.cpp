#include <sender.h>

#include <cassert>
#include <cstring>
#include <fstream>
#include <fcntl.h>
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
    m_src_file_fd(-1),
    m_dest_hostname(nullptr),
    m_dest_port(nullptr),
    m_addrinfo(nullptr)
{
    if (setup_src_file() != 0) {
        std::cerr << "Error opening source file for transfer\n";
        ::exit(EXIT_FAILURE);
    }

    if (read_addrinfo() != 0) {
        std::cerr << "Error reading address info from " << CHANNEL_INFO_FILENAME << '\n';
        ::exit(EXIT_FAILURE);
    }

    if (setup_socket() != 0) {
        std::cerr << "Error creating socket\n";
        ::exit(EXIT_FAILURE);
    }

    send_src_file();
}

Sender::~Sender()
{
    if (m_sock_fd != -1) {
        ::close(m_sock_fd);
        m_sock_fd = -1;
    }

    if (m_src_file_fd != -1) {
        ::close(m_src_file_fd);
        m_src_file_fd = -1;
    }

    if (m_dest_hostname != nullptr) {
        delete[] m_dest_hostname;
        m_dest_hostname = nullptr;
    }

    if (m_dest_port != nullptr) {
        delete[] m_dest_port;
        m_dest_port = nullptr;
    }

    if (m_addrinfo != nullptr) {
        ::freeaddrinfo(m_addrinfo);
        m_addrinfo = nullptr;
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
    hints.ai_socktype = SOCK_DGRAM;

    ret = ::getaddrinfo(m_dest_hostname, m_dest_port, &hints, &recvinfo);

    if (ret != 0) {
        std::cerr << "getaddrinfo: " << ::gai_strerror(ret) << '\n';
        ::freeaddrinfo(recvinfo);
        return ret;
    }

    for (m_addrinfo = recvinfo; m_addrinfo != nullptr; m_addrinfo = m_addrinfo->ai_next) {
        m_sock_fd = ::socket(m_addrinfo->ai_family, m_addrinfo->ai_socktype, m_addrinfo->ai_protocol);

        if (m_sock_fd == -1) {
            continue;
        }

        // If we've successfully connected, don't need to keep iterating through
        break;
    }

    if (m_addrinfo == nullptr) {
        std::cerr << "Could not connect to any addrinfo from getaddrinfo()\n";
        m_sock_fd = -1;
        ::freeaddrinfo(recvinfo);
        return -1;
    }

    return 0;
}

int Sender::setup_src_file()
{
    m_src_file_fd = ::open(m_filename.c_str(), O_RDONLY);

    if (m_src_file_fd == -1) {
        std::perror("open");
        return -1;
    }

    return 0;
}

int Sender::send_src_file()
{
    if (m_src_file_fd == -1) {
        std::cerr << "Can't send file without open fd\n";
        return -1;
    }

    off_t offset = 0;
    ssize_t bytes_read = 0;
    unsigned char payload[MAX_PAYLOAD_SIZE];
    uint32_t seq_num = 0;
    Packet reply = create_invalid_packet();
    int ret = -1;

    while (true) {
        // Read MAX_PAYLOAD_SIZE chunks of the source file until we get to the end
        bytes_read = ::pread(m_src_file_fd, &payload[0], MAX_PAYLOAD_SIZE, offset);

        if (bytes_read == -1) {
            std::perror("pread");
            return -2;
        } else if (bytes_read == 0) {
            // EOF
            break;
        }

        ret = send_packet(
            m_sock_fd,
            create_packet(DAT, seq_num, &payload[0], bytes_read),
            m_addrinfo->ai_addr,
            m_addrinfo->ai_addrlen
        );

        if (ret != 0) {
            std::cerr << "Error sending packet\n";
            return ret;
        }

        reply = create_invalid_packet();
        ret = receive_packet(m_sock_fd, reply, nullptr, nullptr);

        if (ret != 0) {
            std::cerr << "Error receiving reply packet\n";
            return ret;
        }

        if (reply.type != ACK) {
            std::cerr << "Did not receive ACK\n";
            return -3;
        }

        seq_num = (seq_num + 1) % MAX_SEQ_NUM;
        offset += bytes_read;
    }

    ret = send_packet(
        m_sock_fd,
        create_eot(),
        m_addrinfo->ai_addr,
        m_addrinfo->ai_addrlen
    );

    if (ret != 0) {
        std::cerr << "Error sending EOT after file transmission\n";
        return ret;
    }

    reply = create_invalid_packet();
    ret = receive_packet(m_sock_fd, reply, nullptr, nullptr);

    if (ret != 0) {
        std::cerr << "Error receiving reply after sending EOT\n";
        return ret;
    }

    if (reply.type != EOT) {
        std::cerr << "Did not receive EOT in reply\n";
        return -4;
    }

    return 0;
}

} // namespace a3
