#ifndef PAXOS_H
#define PAXOS_H

#include "chat.h"
//#include "tcpserver.h"
#include "boost/asio.hpp"
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace messenger {
enum paxos_states { free = 0, busy = 1 };
enum paxos_errors { ok = 0, paxos_busy = 1, hash_mismatch = 2, force_stop = 3 };

class paxos {
public:
  paxos(boost::asio::io_context &, chat &,
        std::vector<std::pair<std::string, uint32_t>> &);

  paxos_states get_state() { return state; }

  uint32_t start_accept(std::shared_ptr<messenger::chat_event>, uint64_t);

  void accept_promise(std::string id, std::string hash);

  void stop();

  template <typename T> void loop_through_users(T processor) {
    for (auto &i : participants) {
      processor(i);
    }
  }

  bool correct_action(std::shared_ptr<messenger::chat_event>);

private:
  void handler(uint32_t);

  void clear();

  std::map<std::string, uint32_t> participants; // id, weight
  std::map<std::string, bool> registered_members;
  std::map<std::string, uint32_t> data_version_weights;
  std::string most_common_value = "";
  uint32_t total_weight = 0;
  std::shared_ptr<messenger::chat_event> c_value;
  paxos_states state = paxos_states::free;
  chat &c_chat;
  std::mutex paxos_locker;
  boost::asio::steady_timer timer;
};
} // namespace messenger
#endif
