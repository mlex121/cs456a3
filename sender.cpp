#include <sender.h>

#include <fstream>
#include <iostream>

#include <shared.h>

namespace a3 {

const char *CHANNEL_INFO_FILENAME = "channelInfo";

Sender::Sender(uint32_t timeout, const std::string &filename) :
    m_timeout(timeout),
    m_filename(filename),
    m_dest_hostname(nullptr),
    m_dest_port(nullptr)
{}

int Sender::execute()
{
    std::cout << m_timeout << '\n';
    std::cout << m_filename << '\n';

    if (read_addrinfo() == 0) {
        std::cout << std::string(m_dest_hostname) << '\n';
        std::cout << std::string(m_dest_port) << '\n';
    }

    return ProgramErrorNone;
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
        return -3;
    }

    std::strncpy(m_dest_port, dest_port.c_str(), dest_port.size());
    return 0;
}

} // namespace a3
