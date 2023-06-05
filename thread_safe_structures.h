#ifndef THREAD_SAFE_STRUCTS_H
#define THREAD_SAFE_STRUCTS_H
#include "chat_info_list_fwd.h"
#include <map>
#include <memory>
#include <mutex>
#include <utility>

template <typename key_t_arg, typename val_t_arg> class thread_safe_map {
public:
  template <typename... construct_args>
  void add(const key_t_arg &key, construct_args &&...args) {
    std::unique_lock ul{mutex_d};
    map_d.emplace(key, std::forward<construct_args>(args)...);
  }

  val_t_arg get(const key_t_arg &key) {
    std::unique_lock ul{mutex_d};
    return map_d[key];
  }

private:
  std::map<key_t_arg, val_t_arg> map_d;
  std::mutex mutex_d;
};

#endif