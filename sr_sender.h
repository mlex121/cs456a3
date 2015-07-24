#ifndef SR_SENDER_H
#define SR_SENDER_H

#include <sender.h>

namespace a3 {

class SRSender : public Sender {
public:
    SRSender(uint32_t timeout, const std::string &filename);
protected:
private:
};

} // namespace a3

#endif // SR_SENDER_H
