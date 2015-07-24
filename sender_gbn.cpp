#include <sender.h>

#include <iostream>
#include <unistd.h>

#include <shared.h>

namespace a3 {

int Sender::upload_file()
{
    if (m_src_file_fd == -1) {
        std::cerr << "Can't send file without open fd\n";
        return -1;
    }

    off_t offset = 0;
    ssize_t bytes_read = 0;
    unsigned char payload[MAX_PAYLOAD_SIZE];
    uint32_t seq_num = 0;
    Packet reply = create_invalid_packet();
    int ret = -1;

    while (true) {
        // Read MAX_PAYLOAD_SIZE chunks of the source file until we get to the end
        bytes_read = ::pread(m_src_file_fd, &payload[0], MAX_PAYLOAD_SIZE, offset);

        if (bytes_read == -1) {
            std::perror("pread");
            return -2;
        } else if (bytes_read == 0) {
            // EOF
            break;
        }

        ret = send_packet(
            m_sock_fd,
            create_packet(DAT, seq_num, &payload[0], bytes_read),
            m_addrinfo->ai_addr,
            m_addrinfo->ai_addrlen
        );

        if (ret != 0) {
            std::cerr << "Error sending packet\n";
            return ret;
        }

        reply = create_invalid_packet();
        ret = receive_packet(m_sock_fd, reply, nullptr, nullptr);

        if (ret != 0) {
            std::cerr << "Error receiving reply packet\n";
            return ret;
        }

        if (reply.type != ACK) {
            std::cerr << "Did not receive ACK\n";
            return -3;
        }

        seq_num = (seq_num + 1) % MAX_SEQ_NUM;
        offset += bytes_read;
    }

    ret = send_packet(
        m_sock_fd,
        create_eot(),
        m_addrinfo->ai_addr,
        m_addrinfo->ai_addrlen
    );

    if (ret != 0) {
        std::cerr << "Error sending EOT after file transmission\n";
        return ret;
    }

    reply = create_invalid_packet();
    ret = receive_packet(m_sock_fd, reply, nullptr, nullptr);

    if (ret != 0) {
        std::cerr << "Error receiving reply after sending EOT\n";
        return ret;
    }

    if (reply.type != EOT) {
        std::cerr << "Did not receive EOT in reply\n";
        return -4;
    }

    return 0;
}

} // namespace a3

