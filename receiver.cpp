#include <receiver.h>

#include <climits>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace a3 {

const char *RECV_INFO_FILENAME = "recvInfo";

// The maximum port length (in terms of string length) isn't provided by the
// system, so we calculate it ourselves.
const size_t MAX_PORT_LEN = std::max(
    std::to_string(std::numeric_limits<in_port_t>::max()).size(),
    std::to_string(std::numeric_limits<in_port_t>::min()).size()
);

// The maximum number of pending connections for listen(2). Usually should be 1.
const int MAX_CONNECTIONS = 1;

Receiver::Receiver(const std::string &filename) :
    m_filename(filename),
    m_hostname(nullptr),
    m_port(nullptr),
    m_sock_fd(-1)
{
    // Create the socket and bind to it
    if (setup_socket() != 0) {
        std::cerr << "Error setting up the socket" << '\n';
        ::exit(EXIT_FAILURE);
    }

    // Fill in our hostname and port
    if (setup_addrinfo() != 0) {
        std::cerr << "Error filling in hostname and port" << '\n';
        ::exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(m_sock_fd, MAX_CONNECTIONS) == -1) {
        ::perror("listen");
        ::exit(EXIT_FAILURE);
    }

    // Write the hostname and port to file
    if (write_addrinfo() != 0) {
        std::cerr << "Error writing address info to " << RECV_INFO_FILENAME << '\n';
        ::exit(EXIT_FAILURE);
    }
}

Receiver::~Receiver()
{
    if (m_hostname != nullptr) {
        delete[] m_hostname;
        m_hostname = nullptr;
    }

    if (m_port != nullptr) {
        delete[] m_port;
        m_port = nullptr;
    }
}

int Receiver::setup_socket()
{
    // Create a socket
    m_sock_fd = ::socket(AF_INET, SOCK_STREAM, 0);

    if (m_sock_fd == -1) {
        ::perror("socket");
        return -1;
    }

    // Initialize fields to 0
    struct sockaddr_in recvaddr = {};

    // Miscellaneous address settings
    recvaddr.sin_family = AF_INET;
    recvaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Take any assigned port
    recvaddr.sin_port = htons(0);

    // Bind to the socket we created
    if (::bind(m_sock_fd, (struct sockaddr *)&recvaddr, sizeof(recvaddr)) == -1) {
        ::perror("bind");
        return -2;
    }

    return 0;
}

int Receiver::setup_addrinfo()
{
    struct sockaddr_in recvinfo = {};
    socklen_t recvinfo_len = sizeof(recvinfo);

    // Fill in recvinfo
    if (::getsockname(m_sock_fd, (struct sockaddr *)&recvinfo, &recvinfo_len) == -1) {
        ::perror("getsockname");
        return -1;
    }

    m_port = new char[MAX_PORT_LEN];

    if (m_port == nullptr) {
        std::cerr << "Could not allocate port buffer" << '\n';
        return -2;
    }

    // Copy the port from recvinfo
    std::strncpy(m_port, std::to_string(ntohs(recvinfo.sin_port)).c_str(), MAX_PORT_LEN);

    m_hostname = new char[_POSIX_HOST_NAME_MAX];

    if (m_hostname == nullptr) {
        std::cerr << "Could not allocate hostname buffer" << '\n';
        delete[] m_port;
        m_port = nullptr;
        return -3;
    }

    // Copy the current hostname
    if (::gethostname(m_hostname, _POSIX_HOST_NAME_MAX) == -1) {
        ::perror("gethostname");
        delete[] m_port;
        m_port = nullptr;
        return -4;
    }

    return 0;
}

int Receiver::write_addrinfo()
{
    // Can't write empty hostname/port
    if (m_hostname == nullptr || m_port == nullptr) {
        return -1;
    }

    // We set trunc because we want to replace the contents of the info file
    std::ofstream file(RECV_INFO_FILENAME, std::ofstream::out | std::ofstream::trunc);

    if (!(file << m_hostname << ' ' << m_port << '\n')) {
        return -2;
    }

    return 0;
}

} // namespace a3
