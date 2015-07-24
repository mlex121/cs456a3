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

} // namespace a3

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " <timeout> <filename>\n";
        return EXIT_FAILURE;
    }

    uint32_t timeout = std::strtoul(argv[1], nullptr, 10);
    std::string filename(argv[2]);
    a3::Sender sender(timeout, filename);
    sender.upload_file();

    return 0;
}
