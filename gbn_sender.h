#ifndef GBN_SENDER_H
#define GBN_SENDER_H

#include <sender.h>

namespace a3 {

class GBNSender : public Sender {
public:
    GBNSender(uint32_t timeout, const std::string &filename);
protected:
private:
};

} // namespace a3

#endif // GBN_SENDER_H
