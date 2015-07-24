#include <sender_sr.h>

#include <iostream>

namespace a3 {

SRSender::SRSender(uint32_t timeout, const std::string &filename) :
    Sender(timeout, filename) {}

void SRSender::upload_file()
{
}

} // namespace a3

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " <timeout> <filename>\n";
        return EXIT_FAILURE;
    }

    uint32_t timeout = std::strtoul(argv[1], nullptr, 10);
    std::string filename(argv[2]);
    a3::SRSender sender(timeout, filename);
    sender.upload_file();

    return 0;
}
