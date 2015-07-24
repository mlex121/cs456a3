#ifndef SENDER_SR_H
#define SENDER_SR_H

#include <sender.h>

namespace a3 {

class SRSender : public Sender {
public:
    explicit SRSender(uint32_t timeout, const std::string &filename);

    virtual void upload_file() override final;
protected:
private:

};

} // namespace a3

#endif // SENDER_SR_H
