#ifndef SENDER_GBN_H
#define SENDER_GBN_H

#include <deque>
#include <unistd.h>

#include <sender.h>

namespace a3 {

class GBNSender : public Sender {
public:
    explicit GBNSender(uint32_t timeout, const std::string &filename);

    virtual void upload_file();
    void send_frame_window();
    void wait_for_ack();
protected:
private:
    off_t m_offset;
    uint32_t m_seq_num;
    uint32_t m_send_base;
    bool m_done_reading;
    std::deque<Packet> m_frame_window;

    void reload_frame_window(size_t num_packets);
};

} // namespace a3

#endif // SENDER_GBN_H
