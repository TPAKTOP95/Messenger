#ifndef CHAT_INFO_LIST_H
#define CHAT_INFO_LIST_H

#include "boost/asio.hpp"
#include "chat.h"
#include "paxos.h"
#include "thread_safe_structures.h"
#include <string>

namespace chat_info {
class chat_info_list {
public:
  /*
  bool add(boost::asio::io_context &io, std::string my_id, std::string chat_id,
           std::vector<std::pair<std::string, uint32_t>>
               &participants_vec) { // can add if no chat_instances in use and
                                    // paxos stoped
    std::scoped_lock lock(paxos_list.mutex_d, chat_list.mutex_d);
    auto &chat_inst = chat_list.get(chat_id);
    auto &paxos_inst = paxos_list.get(chat_id);
    if ((chat_mask == nullptr) &&
        (paxos_mask == nullptr)) { // must be symmetrically nullptr/!nullptr
      chat_list.add(chat_id, my_id, chat_id);
      paxos_list.add(chat_id, io, *(chat_list.get(chat_id)), participants_vec);
      return true;
    } else {
      if ((chat_inst.use_count() == 2) &&
          (paxos_inst.use_count() == 2) && // map ptr+ chat_inst
          (paxos_inst->get_state() == messenger::paxos_states::free)) {
        chat_list.add(chat_id, my_id, chat_id);
        paxos_list.add(chat_id, io, *(chat_list.get(chat_id)),
                       participants_vec);
        return true;
      } else {
        return false;
      }
    }
  }

  std::pair<std::shared_ptr<messenger::chat>, std::shared_ptr<messenger::paxos>>
  get(std::string chat_id) {
    std::scoped_lock lock(paxos_list.mutex_d, chat_list.mutex_d);
    return {chat_list.get(chat_id), paxos_list.get(chat_id)};
  }

private:
  thread_safe_map<std::string, messenger::paxos> paxos_list;
  thread_safe_map<std::string, messenger::chat> chat_list;
  */
};

} // namespace chat_info
#endif