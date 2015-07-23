#ifndef RECEIVER_H
#define RECEIVER_H

#include <string>

namespace a3 {

extern const char *RECV_INFO_FILENAME;

class Receiver {
public:
    explicit Receiver(const std::string &filename);
    int execute();
protected:
    std::string m_filename;
private:
    int write_addrinfo();

    const char *m_hostname;
    const char *m_port;
};

} // namespace a3

#endif // RECEIVER_H
