#include <receiver.h>

#include <iostream>

#include <shared.h>

namespace a3 {

int Receiver::download_file()
{
    Packet packet = create_invalid_packet();
    struct sockaddr_storage servaddr = {};
    socklen_t servaddr_len = sizeof(servaddr);

    while (true) {
        if (receive_packet(m_sock_fd, packet, (struct sockaddr *)&servaddr, &servaddr_len) == 0) {
            switch (packet.type) {
                case DAT:
                    write_to_dest_file(packet);
                    send_packet(m_sock_fd, create_ack(packet.seq_num), (struct sockaddr *)&servaddr, servaddr_len);
                    break;
                case ACK:
                    break;
                case EOT:
                    send_packet(m_sock_fd, create_eot(), (struct sockaddr *)&servaddr, servaddr_len);
                    ::exit(0);
                    break;
                case INVALID_PACKET:
                    break;
            }
        }
    }

    return 0;
}

} // namespace a3
