#ifndef SENDER_H
#define SENDER_H

#include <string>
#include <cstdint>
#include <shared.h>
#include <netdb.h>

namespace a3 {

extern const char *CHANNEL_INFO_FILENAME;

class Sender {
public:
    explicit Sender(uint32_t timeout, const std::string &filename);
    Sender(Sender &s) = delete;
    Sender(Sender &&s) = delete;
    Sender &operator=(const Sender &s) = delete;
    Sender &&operator=(const Sender &&s) = delete;
    virtual ~Sender();
protected:
    int send_src_file();

    uint32_t m_timeout;
    std::string m_filename;
private:
    int setup_socket();
    int setup_src_file();
    int read_addrinfo();

    int m_sock_fd;
    int m_src_file_fd;
    char *m_dest_hostname;
    char *m_dest_port;
    struct addrinfo *m_addrinfo;
};

} // namespace a3

#endif // SENDER_H
