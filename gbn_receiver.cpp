#include <gbn_receiver.h>

#include <shared.h>

namespace a3 {

} // namespace a3

int main(int argc, char **argv)
{
    if (argc != 2) {
        return EXIT_FAILURE;
    }

    std::string filename = std::string(argv[1]);
    a3::GBNReceiver receiver(filename);

    return 0;
}
