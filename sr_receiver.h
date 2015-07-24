#ifndef SR_RECEIVER_H
#define SR_RECEIVER_H

#include <receiver.h>

namespace a3 {

class SRReceiver : public Receiver {
public:
    explicit SRReceiver(const std::string &filename);
protected:
private:
};

} // namespace a3

#endif // SR_RECEIVER_H
