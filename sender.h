#ifndef SENDER_H
#define SENDER_H

#include <string>
#include <cstdint>

#include <shared.h>

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
    int send_packet(Packet packet);
    Packet receive_reply();

    uint32_t m_timeout;
    std::string m_filename;
private:
    int setup_socket();
    int read_addrinfo();

    int m_sock_fd;
    char *m_dest_hostname;
    char *m_dest_port;
};

} // namespace a3

#endif // SENDER_H
