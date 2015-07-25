#include <receiver.h>

#include <array>
#include <iostream>

#include <shared.h>

namespace a3 {

int Receiver::download_file()
{
    Packet packet = create_invalid_packet();
    struct sockaddr_storage servaddr = {};
    socklen_t servaddr_len = sizeof(servaddr);
    uint32_t req_num = 0;
    std::array<Packet, WINDOW_SIZE> window;
    bool has_processed_packet = false;

    window.fill(create_invalid_packet());

    while (true) {
        if (receive_packet(m_sock_fd, packet, (struct sockaddr *)&servaddr, &servaddr_len) != 0) {
            std::cerr << "Error receiving packet\n";
            return -1;
        }

        switch (packet.type) {
            case DAT:
            {
                int32_t dist = distance(req_num, packet.seq_num);

                // The packet fits in our window, we can buffer it
                if (0 <= dist && dist < (int32_t)window.size()) {
                    std::cout << "BUFFERING PACKET IN SLOT " << dist << '\n';
                    window[dist] = packet;
                }

                break;
            }
            case ACK:
            {
                break;
            }
            case INVALID_PACKET:
            {
                break;
            }
            case EOT:
            {
                // On EOT, we reply with an EOT and exit
                send_packet(m_sock_fd, create_eot(), (struct sockaddr *)&servaddr, servaddr_len);
                return 0;
            }
        }

        uint32_t processed = 0;

        // Buffer processing
        for (size_t i = 0; i < window.size(); ++i) {
            Packet p = window[i];

            // Can only do out-of-order packets if contiguous
            if (p.type != DAT) {
                break;
            }

            // ACK each packet we process and write them to file
            send_packet(m_sock_fd, create_ack(p.seq_num), (struct sockaddr *)&servaddr, servaddr_len);
            write_to_dest_file(p);
            processed++;
            has_processed_packet = true;
        }

        // Shift the buffer over for each processed packet
        for (size_t i = processed; i < window.size(); ++i) {
            window[i - processed] = window[i];
        }

        // Fill in the remainder
        for (size_t i = window.size() - processed; i < window.size(); ++i) {
            window[i] = create_invalid_packet();
        }

        // Update our requested sequence number
        req_num = (req_num + processed) % SEQ_NUM_BASE;

        // Always send an ACK with our most-recently processed packet, but only
        // if we've actually processed a packet before
        if (has_processed_packet) {
            uint32_t most_recent_seq_num = (req_num + (SEQ_NUM_BASE - 1)) % SEQ_NUM_BASE;
            send_packet(m_sock_fd, create_ack(most_recent_seq_num), (struct sockaddr *)&servaddr, servaddr_len);
        }
    }

    // Should not reach here
    return -2;
}

} // namespace a3
