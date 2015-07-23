#include <sender.h>

namespace a3 {

Sender::Sender(uint32_t timeout, const std::string &filename) :
    m_timeout(timeout),
    m_filename(filename)
{}

} // namespace a3
