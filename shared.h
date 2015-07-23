#ifndef SHARED_H
#define SHARED_H

#include <cstdint>
#include <cstdlib>
#include <string>

namespace a3 {

enum ProgramErrors : int {
    ProgramErrorNone = 0,
    ProgramErrorInvalidArguments,
};

static const size_t MAX_PAYLOAD_SIZE = 500;

enum PacketType : uint32_t {
    DAT = 0, // Data
    ACK = 1, // Acknowledge
    EOT = 2, // End-of-transmission
};

enum TransferDirection : uint32_t {
    SEND,
    RECV,
};

typedef struct {
    PacketType type;
    uint32_t seq_num;
    uint32_t length;
    size_t payload_size;
    unsigned char payload[MAX_PAYLOAD_SIZE];
} Packet;

static const size_t PACKET_HEADER_SIZE = sizeof(PacketType) + (2 * sizeof(uint32_t));
static const size_t MAX_PACKET_SIZE = PACKET_HEADER_SIZE + MAX_PAYLOAD_SIZE;

/**
 * Creates a Packet struct with the given type, sequence number, and payload,
 * and fills in the length of the resulting packet.
 *
 * @param  type         Packet type
 * @param  seq_num      Modulo 256 sequence number of the packet (or the packet
 *                      being acknowledged, for ACK packets)
 * @param  payload      The byte payload of the packet
 * @param  payload_size The size of the payload, in bytes
 *
 * @return              The created Packet struct
 */
Packet create_packet(PacketType type, uint32_t seq_num, const unsigned char *payload, size_t payload_size);

/**
 * Convenience method to create an ACK for a given sequence number.
 *
 * @param  seq_num Sequence number being acknowledged
 *
 * @return         An ACK packet
 */
Packet create_ack(uint32_t seq_num);

/**
 * Convenience method to create an end-of-transmission packet.
 *
 * @return An end-of-transmission packet
 */
Packet create_eot();

/**
 * Compares the packets and returns whether or not they're the same.
 *
 * @param  p1 The first packet to compare
 * @param  p2 The second packet to compare
 *
 * @return    True if the packets are equal, false if they are not equal
 */
bool equal_packets(const Packet &p1, const Packet &p2);

/**
 * Allocates a byte array and serializes the packet into the array. Integers are
 * serialized in big-endian order.
 *
 * @param  packet The packet to serialize
 *
 * @return        A newly allocated byte array containing the serialized packet,
 *                or nullptr if it could not be created.
 */
unsigned char *serialize_packet(Packet packet);

/**
 * Creates a Packet struct from the given serialized packet (byte array). Must
 * be a valid packet, since we use bytes 4-8 to get the length.
 *
 * @param  serialized The serialized packet, must not be nullptr
 *
 * @return            The created Packet struct
 */
Packet deserialize_packet(const unsigned char *serialized);

/**
 * Creates a log string from a packet in the following format:
 * PKT {SEND|RECV} {DAT|ACK|EOT} <sequence number> <total length>
 *
 * @param  packet    The packet to create a log string from
 * @param  direction Whether the packet was sent or received
 *
 * @return           The log line
 */
std::string packet_to_log_string(Packet packet, TransferDirection direction);

} // namespace a3

#endif // SHARED_H
