#ifndef NETWORK_TYPES_H
#define NETWORK_TYPES_H

#include "chat.h"
#include <cryptopp/rsa.h>

namespace messenger {
const int IDLEN = 513;
enum network_type {
  dialog_text = 0,
  paxos_notif = 1,
  paxos_push = 2,
  chat_sync = 3
};

namespace network_packets {

struct dialog_text {
  CryptoPP::RSA::PublicKey id;
  std::string text;
};

struct paxos_notif_packet {
  char chat_id[IDLEN] = "";
  char id[IDLEN] = "";
  char hash[IDLEN] = "";
};

struct paxos_push_packet {
  char chat_id[IDLEN] = "";
  char id[IDLEN] = "";
  uint64_t time = 0;
  std::shared_ptr<messenger::chat_event> c_event;
};

struct request_chat_hash {
  char chat_id[IDLEN] = "";
  char id[IDLEN] = "";
  uint64_t time = 0;
};

} // namespace network_packets
} // namespace messenger

#endif