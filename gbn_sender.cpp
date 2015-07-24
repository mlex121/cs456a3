#include <gbn_sender.h>

#include <shared.h>

namespace a3 {

} // namespace a3

int main(int argc, char **argv)
{
    if (argc != 3) {
        return EXIT_FAILURE;
    }

    uint32_t timeout = std::strtoul(argv[1], nullptr, 10);
    std::string filename(argv[2]);
    a3::GBNSender sender(timeout, filename);

    return 0;
}
