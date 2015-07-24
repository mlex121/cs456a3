#ifndef SENDER_H
#define SENDER_H

#include <string>
#include <cstdint>
#include <netdb.h>

#include <shared.h>

namespace a3 {

extern const char *CHANNEL_INFO_FILENAME;

class Sender {
public:
    explicit Sender(uint32_t timeout, const std::string &filename);
    virtual ~Sender();

    int upload_file();
protected:
private:
    Sender(Sender &s) = delete;
    Sender(Sender &&s) = delete;
    Sender &operator=(const Sender &s) = delete;
    Sender &&operator=(const Sender &&s) = delete;

    int setup_socket();
    int setup_src_file();
    int read_addrinfo();

    uint32_t m_timeout;
    std::string m_filename;
    int m_sock_fd;
    int m_src_file_fd;
    char *m_dest_hostname;
    char *m_dest_port;
    struct addrinfo *m_addrinfo;
};

} // namespace a3

#endif // SENDER_H
