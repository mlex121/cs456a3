#include <receiver.h>

#include <fstream>

namespace a3 {

const char *RECV_INFO_FILENAME = "recvInfo";

Receiver::Receiver(const std::string &filename) :
    m_filename(filename),
    m_hostname(nullptr),
    m_port(nullptr)
{}

int Receiver::execute()
{
    write_addrinfo();
    return 0;
}

int Receiver::write_addrinfo()
{
    if (m_hostname == nullptr || m_port == nullptr) {
        return -1;
    }

    std::ofstream file(RECV_INFO_FILENAME, std::ofstream::out | std::ofstream::trunc);

    if (!(file << m_hostname << ' ' << m_port << '\n')) {
        return -2;
    }

    return 0;
}

} // namespace a3
