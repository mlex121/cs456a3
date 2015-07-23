#ifndef SENDER_H
#define SENDER_H

#include <string>
#include <cstdint>

namespace a3 {

class Sender {
private:
protected:
    uint32_t m_timeout;
    std::string m_filename;
public:
    explicit Sender(uint32_t timeout, const std::string &filename);
    virtual int execute() = 0;
};

} // namespace a3

#endif // SENDER_H
