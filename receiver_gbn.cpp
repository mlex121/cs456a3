#include <receiver.h>

#include <iostream>

#include <shared.h>

namespace a3 {

int Receiver::download_file()
{
    Packet packet = create_invalid_packet();
    struct sockaddr_storage servaddr = {};
    socklen_t servaddr_len = sizeof(servaddr);
    uint32_t req_num = 0;

    while (true) {
        if (receive_packet(m_sock_fd, packet, (struct sockaddr *)&servaddr, &servaddr_len) != 0) {
            std::cerr << "Error receiving packet\n";
            return -1;
        }

        switch (packet.type) {
            case DAT:
            {
                // Only accept the expected sequence number
                if (packet.seq_num == req_num) {
                    send_packet(m_sock_fd, create_ack(packet.seq_num), (struct sockaddr *)&servaddr, servaddr_len);
                    write_to_dest_file(packet);
                    req_num = (req_num + 1) % MAX_SEQ_NUM;
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
    }

    // Should not reach here
    return -2;
}

} // namespace a3
