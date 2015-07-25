#include <sender_gbn.h>

#include <cmath>
#include <deque>
#include <iostream>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <shared.h>

namespace a3 {

GBNSender::GBNSender(uint32_t timeout, const std::string &filename) :
    Sender(timeout, filename),
    m_offset(0),
    m_seq_num(0),
    m_send_base(0),
    m_packets_sent(0),
    m_done_reading(false),
    m_frame_window()
{}

// Ugly hack I need to do to access methods through the signal handler
GBNSender *sender = nullptr;

static void handler(int sig)
{
    (void)sig;

    // We timed out trying to receive an ACK reply, so send the frames again
    if (sender != nullptr) {
        sender->send_frame_window();

        while (true) {
            sender->wait_for_ack();
        }
    }
}

void GBNSender::upload_file()
{
    struct sigaction sa = {};

    // Register handler(int) as the SIGALRM handler
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NODEFER;
    sa.sa_handler = handler;

    if (::sigaction(SIGALRM, &sa, nullptr) == -1) {
        std::perror("sigaction");
        ::exit(EXIT_FAILURE);
    }

    // Buffer the first window of packets of data for the file and send it
    reload_frame_window(WINDOW_SIZE);
    send_frame_window();

    while (true) {
        // wait_for_ack() will handle reloading and sending
        wait_for_ack();
    }
}

void GBNSender::reload_frame_window(size_t num_packets)
{
    if (m_done_reading) {
        return;
    }

    // Dequeue packets to make room for new ones
    for (size_t i = 0; i < num_packets; ++i) {
        if (m_frame_window.empty()) {
            break;
        }

        m_frame_window.pop_front();
    }

    ssize_t bytes_read = 0;
    unsigned char payload[MAX_PAYLOAD_SIZE];

    for (size_t i = 0; i < num_packets; ++i) {
        // Read in a full payload
        bytes_read = ::pread(m_src_file_fd, &payload[0], MAX_PAYLOAD_SIZE, m_offset);
        m_offset += bytes_read;

        if (bytes_read == -1) {
            std::perror("pread");
            ::exit(EXIT_FAILURE);
        }

        // pread() returns 0 on EOF
        if (bytes_read == 0) {
            m_done_reading = true;
            return;
        }

        // Add to our frame window
        m_frame_window.push_back(create_packet(DAT, m_seq_num, &payload[0], bytes_read));
        m_seq_num = (m_seq_num + 1) % SEQ_NUM_BASE;
    }
}

void GBNSender::send_frame_window()
{
    for (size_t i = 0; i < m_frame_window.size(); ++i) {
        if (send_packet(m_sock_fd, m_frame_window[i], m_addrinfo->ai_addr, m_addrinfo->ai_addrlen) != 0) {
            std::cerr << "Error sending packet\n";
            ::exit(EXIT_FAILURE);
        }
    }
}

void GBNSender::wait_for_ack()
{
    int ret = -1;

    // Cancel the previous ualarm before calling a new one
    ::ualarm(0, 0);
    ::ualarm(m_timeout * 1000, 0);

    Packet packet = create_invalid_packet();
    ret = receive_packet(m_sock_fd, packet, nullptr, nullptr);

    // Cancel the alarm receive_packet returned
    ::ualarm(0, 0);

    // ret == 1 means receive_packet got interrupted by our signal handler
    if (ret == 1) {
        return;
    }

    if (ret != 0) {
        std::cerr << "Error receiving reply ACK\n";
        ::exit(EXIT_FAILURE);
    }

    // Ignore other types of packets
    if (packet.type == ACK) {
        int32_t dist = distance(m_send_base, packet.seq_num);

        if (dist >= 0) {
            // If the distance is 0, our first packet was acked, and so on
            uint32_t packets_acked = dist + 1;

            // Need to promote the values to floats before dividing, otherwise
            // the decimal gets truncated
            uint32_t total_packets = std::ceil((1.0f * m_offset) / MAX_PAYLOAD_SIZE);

            m_packets_sent += packets_acked;
            m_send_base = (m_send_base + packets_acked) % SEQ_NUM_BASE;

            if (m_done_reading && m_packets_sent >= total_packets) {
                // Done reading in our file and all packets have been sent
                ret = end_of_transfer();

                if (ret != 0) {
                    std::cerr << "Failed to end transfer\n";
                }

                ::exit(0);
            } else {
                // Not done transferring
                reload_frame_window(packets_acked);
                send_frame_window();
            }
        }
    }
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
    a3::sender = new a3::GBNSender(timeout, filename);
    a3::sender->upload_file();
    delete a3::sender;

    return 0;
}
