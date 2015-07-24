#ifndef RECEIVER_H
#define RECEIVER_H

#include <string>

#include <shared.h>

namespace a3 {

extern const char *RECV_INFO_FILENAME;

class Receiver {
public:
    explicit Receiver(const std::string &filename);
    virtual ~Receiver();

    int download_file();
protected:
private:
    Receiver(Receiver &s) = delete;
    Receiver(Receiver &&s) = delete;
    Receiver &operator=(const Receiver &s) = delete;
    Receiver &&operator=(const Receiver &&s) = delete;

    int setup_socket();
    int setup_addrinfo();
    int setup_dest_file();
    int write_addrinfo() const;
    int write_to_dest_file(const unsigned char *buffer, size_t length) const;
    int write_to_dest_file(const Packet &packet) const;

    std::string m_filename;
    int m_sock_fd;
    int m_dest_file_fd;
    char *m_hostname;
    char *m_port;
};

} // namespace a3

#endif // RECEIVER_H
