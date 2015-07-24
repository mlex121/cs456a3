#ifndef RECEIVER_H
#define RECEIVER_H

#include <string>

namespace a3 {

extern const char *RECV_INFO_FILENAME;

class Receiver {
public:
    explicit Receiver(const std::string &filename);
    Receiver(Receiver &s) = delete;
    Receiver(Receiver &&s) = delete;
    Receiver &operator=(const Receiver &s) = delete;
    Receiver &&operator=(const Receiver &&s) = delete;
    virtual ~Receiver();
protected:
    std::string m_filename;
private:
    int setup_socket();
    int setup_addrinfo();
    int write_addrinfo();

    int m_sock_fd;
    char *m_hostname;
    char *m_port;
};

} // namespace a3

#endif // RECEIVER_H
