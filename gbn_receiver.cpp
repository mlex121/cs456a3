#include <gbn_receiver.h>

#include <iostream>

#include <shared.h>

namespace a3 {

GBNReceiver::GBNReceiver(const std::string &filename) :
    Receiver(filename) {}

} // namespace a3

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <filename>\n";
        return EXIT_FAILURE;
    }

    std::string filename = std::string(argv[1]);
    a3::GBNReceiver receiver(filename);
    receiver.receive_file();

    return 0;
}
