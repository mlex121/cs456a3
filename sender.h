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

    virtual void upload_file() = 0;
protected:
    int end_of_transfer();

    uint32_t m_timeout;
    int m_sock_fd;
    int m_src_file_fd;
    struct addrinfo *m_addrinfo;
private:
    Sender(Sender &s) = delete;
    Sender(Sender &&s) = delete;
    Sender &operator=(const Sender &s) = delete;
    Sender &&operator=(const Sender &&s) = delete;

    int setup_src_file();
    int setup_socket();
    int read_addrinfo();

    std::string m_filename;
    char *m_dest_hostname;
    char *m_dest_port;
};

} // namespace a3

#endif // SENDER_H
