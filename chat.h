#ifndef CHAT_H
#define CHAT_H

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "paxos_fwd.h"

namespace messenger {

enum chat_event_types {
  chat_text_type = 1,
  chat_new_user_type = 2,
  transfer_type = 3
};

class chat_event {
public:
  char event_type = 0;
  std::string initiator;
  uint64_t time = 0;
  virtual ~chat_event() {}
};

class chat_text : public chat_event {
public:
  std::string text;
};

class chat_new_user : public chat_event {
public:
  std::string new_user_id;
};

class transfer : public chat_event {
public:
  uint32_t amount = 0;
  std::string recipient;
};

class chat {
public:
  chat(std::string m_id, std::string c_id) : my_id(m_id), chat_id_s(c_id) {}

  std::shared_ptr<chat_event> get(uint64_t i) {
    std::unique_lock ul{locker};
    if (i >= history.size()) {
      return nullptr;
    }
    return history[i];
  }

  void add(std::shared_ptr<chat_event> c_event) {
    std::unique_lock ul{locker};
    history.emplace_back(::std::move(c_event));
    std::cout << __LINE__ << "chat add" << '\n';
  }
  void clear() {
    std::unique_lock ul{locker};
    history.clear();
  }

  std::string new_hash(::std::shared_ptr<chat_event> c_event) {
    return ::std::to_string(history.size() + 1);
  }
  std::string hash() { return ::std::to_string(history.size()); }

  std::string chat_id() { return chat_id_s; }

  std::string get_my_id() { return my_id; }

  friend paxos;

private:
  std::vector<::std::shared_ptr<chat_event>> history;
  std::mutex locker;
  std::string chat_id_s;
  std::string my_id;
  // std::map<std::string, uint32_t> participants;
};

} // namespace messenger

#endif