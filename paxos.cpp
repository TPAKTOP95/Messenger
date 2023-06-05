#include "paxos.h"
namespace messenger {
paxos::paxos(boost::asio::io_context &io, chat &c,
             std::vector<std::pair<std::string, uint32_t>> &participants_vec)
    : c_chat(c), timer(io) {
  for (auto &i : participants_vec) {
    participants[i.first] = i.second;
  }
  for (auto &i : participants) {
    total_weight += i.second;
  }
}

uint32_t paxos::start_accept(std::shared_ptr<messenger::chat_event> msg,
                             uint64_t deadline) {
  std::unique_lock ul{paxos_locker};
  timer.expires_after(std::chrono::milliseconds(deadline));
  timer.async_wait([this](const boost::system::error_code &ec) {
    if (!ec) {
      this->stop();
    }
  });
  if (state == paxos_states::free) {
    state = paxos_states::busy;
    c_value = msg;
    return 0;
  }
  return 1;
}

void paxos::accept_promise(std::string id, std::string hash) {
  std::unique_lock ul{paxos_locker};
  if (state == paxos_states::busy) {
    if (registered_members[id] == false) {
      registered_members[id] = true;
      data_version_weights[hash] += participants[id];
      if (data_version_weights[hash] >
          data_version_weights[most_common_value]) {
        most_common_value = hash;
      }
      if (2 * data_version_weights[most_common_value] > total_weight) {
        if (most_common_value == c_chat.new_hash(c_value)) {
          handler(paxos_errors::ok);
        } else {
          handler(paxos_errors::hash_mismatch);
        }
      }
    }
  }
}

void paxos::stop() {
  std::unique_lock ul{paxos_locker};
  handler(paxos_errors::force_stop);
}

bool paxos::correct_action(std::shared_ptr<messenger::chat_event> msg) {
  std::unique_lock ul{paxos_locker};
  if (participants.find(msg->initiator) != participants.end()) {
    if (msg->event_type == messenger::chat_event_types::chat_text_type) {
      return true;
    }
  }
  return false;
}

void paxos::handler(uint32_t ec) {
  if (ec == paxos_errors::ok) {
    if (c_value->event_type == messenger::chat_new_user_type) {
      auto s = std::dynamic_pointer_cast<messenger::chat_new_user>(c_value);
    }
    c_chat.add(std::move(c_value));
  } else if (ec == paxos_errors::force_stop) {
    std::cout << "paxos was stoped" << std::endl;
  } else if (ec == paxos_errors::hash_mismatch) {
    std::cout << "hash mismatch. Cant push msg" << std::endl;
  }
  clear();
}

void paxos::clear() {
  state = paxos_states::free;
  c_value = std::shared_ptr<messenger::chat_event>(nullptr);
  registered_members.clear();
  data_version_weights.clear();
  most_common_value.clear();
  timer.cancel();
}

} // namespace messenger