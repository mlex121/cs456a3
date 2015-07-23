#ifndef SENDER_H
#define SENDER_H

#include <string>
#include <cstdint>

namespace a3 {

extern const char *CHANNEL_INFO_FILENAME;

class Sender {
public:
    explicit Sender(uint32_t timeout, const std::string &filename);
    int execute();
protected:
    uint32_t m_timeout;
    std::string m_filename;
private:
    int read_addrinfo();

    char *m_dest_hostname;
    char *m_dest_port;
};

} // namespace a3

#endif // SENDER_H
