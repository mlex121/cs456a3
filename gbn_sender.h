#ifndef GBN_SENDER_H
#define GBN_SENDER_H

#include <sender.h>

namespace a3 {

class GBNSender : Sender {
private:
protected:
public:
    using Sender::Sender;
    virtual int execute() override final;
};

} // namespace a3

#endif // GBN_SENDER_H
