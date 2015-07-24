#include <sr_sender.h>

namespace a3 {

SRSender::SRSender(uint32_t timeout, const std::string &filename) :
    Sender(timeout, filename) {}

} // namespace a3

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    return 0;
}
