#ifndef DESERIALIZER_H
#define DESERIALIZER_H

#include "chat.h"
#include "crypto_utils.h"
#include "network_types.h"
#include <memory>
#include <variant>
namespace messenger {

class reader {
public:
  reader(const char *data_, uint64_t len_)
      : data(data_), limit(len_), pointer(0) {}
  bool read_sequentially(char *dts, uint64_t len) {
    if (pointer + len > limit) {
      return false;
    } else {
      std::memcpy(dts, &data[pointer], len);
      pointer += len;
      return true;
    }
  }

  const char *get_pointer() { return &data[pointer]; }
  const char *get_limit() { return &data[limit]; }

private:
  const char *data;
  uint64_t limit;
  uint64_t pointer;
};

class writer {
public:
  writer(char *data_, uint64_t len_) : data(data_), limit(len_), pointer(0) {}
  bool writer_sequentially(const char *src, uint64_t len) {
    if (pointer + len > limit) {
      return false;
    } else {
      std::memcpy(&data[pointer], src, len);
      pointer += len;
      return true;
    }
  }

  uint64_t get_pointer() { return pointer; }
  uint64_t get_limit() { return limit; }

private:
  char *data;
  uint64_t limit;
  uint64_t pointer;
};

class deserializer {
public:
  static std::variant<messenger::network_packets::paxos_notif_packet,
                      messenger::network_packets::paxos_push_packet,
                      network_packets::dialog_text,
                      messenger::network_packets::request_chat_hash>
  deserialize(std::shared_ptr<char[]> data, uint64_t len) {
    auto raw_data = data.get();

    reader data_reader(raw_data, len);
    char type = 0;
    if (!data_reader.read_sequentially(&type, 1)) {
      return std::variant<messenger::network_packets::paxos_notif_packet,
                          messenger::network_packets::paxos_push_packet,
                          network_packets::dialog_text,
                          messenger::network_packets::request_chat_hash>();
    }
    if (type == network_type::dialog_text) { // id+text
      network_packets::dialog_text pack;
      uint64_t key_size = 0, text_size = 0;
      data_reader.read_sequentially((char *)&key_size, sizeof(key_size));
      bool res =
          data_reader.read_sequentially((char *)&text_size, sizeof(text_size));
      if (res && (len == 1 + 2 * sizeof(uint64_t) + key_size + text_size)) {
        pack.id =
            std::move(digital_signature::bytes_to_rsa_key<decltype(pack.id)>(
                (const unsigned char *)data_reader.get_pointer(), key_size));
        pack.text =
            std::string(data_reader.get_pointer() + key_size, text_size);
        return pack;
      }
    } else if (type == network_type::paxos_notif) {
      network_packets::paxos_notif_packet pack;
      data_reader.read_sequentially(pack.chat_id, sizeof(pack.chat_id));
      data_reader.read_sequentially(pack.id, sizeof(pack.id));
      pack.chat_id[IDLEN - 1] = '\0';
      pack.id[IDLEN - 1] = '\0';
      bool total_result =
          data_reader.read_sequentially(pack.hash, sizeof(pack.hash));
      if (total_result) {
        return pack;
      }
    } else if (type ==
               network_type::paxos_push) { // first byte - chat_event_type
      messenger::network_packets::paxos_push_packet pack;
      char event_type;
      data_reader.read_sequentially(&event_type, 1);
      data_reader.read_sequentially(pack.chat_id, sizeof(pack.chat_id));
      data_reader.read_sequentially(pack.id, sizeof(pack.id));
      bool res = data_reader.read_sequentially(
          reinterpret_cast<char *>(&pack.time), sizeof(pack.time));
      if (res) {
        if (event_type == messenger::chat_event_types::chat_text_type) {
          std::shared_ptr<messenger::chat_text> c_event(
              new messenger::chat_text);
          uint64_t string_len =
              data_reader.get_limit() - data_reader.get_pointer();
          std::unique_ptr<char> str(new char[string_len]);
          if (data_reader.read_sequentially(str.get(), string_len)) {
            c_event->event_type = event_type;
            c_event->text = std::string(str.get(), string_len);
            c_event->time = pack.time;
            c_event->initiator = std::string(pack.id);
            pack.c_event = std::move(c_event);
            return pack;
          }
        }
        if (event_type == messenger::chat_event_types::chat_new_user_type) {
          std::shared_ptr<messenger::chat_new_user> c_event(
              new messenger::chat_new_user);
          std::unique_ptr<char> user_id(new char[IDLEN]);
          if (data_reader.read_sequentially(user_id.get(), IDLEN)) {
            c_event->event_type = event_type;
            c_event->time = pack.time;
            c_event->new_user_id = std::string(user_id.get(), IDLEN);
            c_event->initiator = std::string(pack.id, IDLEN);
            pack.c_event = std::move(c_event);
            return pack;
          }
        }
        if (event_type == messenger::chat_event_types::transfer_type) {
          std::shared_ptr<messenger::transfer> c_event(new messenger::transfer);
          uint32_t amount = 0;
          std::unique_ptr<char[]> recipient(new char[IDLEN]);
          data_reader.read_sequentially(reinterpret_cast<char *>(&amount),
                                        sizeof(c_event->amount));
          if (data_reader.read_sequentially(recipient.get(), IDLEN)) {
            c_event->event_type = event_type;
            c_event->time = pack.time;
            c_event->amount = amount;
            c_event->initiator = std::string(pack.id, IDLEN);
            c_event->recipient = std::string(recipient.get(), IDLEN);
            pack.c_event = std::move(c_event);
            return pack;
          }
        }
      }
    } else if (type == messenger::network_type::chat_sync) {
      messenger::network_packets::request_chat_hash pack;
      data_reader.read_sequentially(pack.chat_id, sizeof(pack.chat_id));
      data_reader.read_sequentially(pack.id, sizeof(pack.id));
      if (data_reader.read_sequentially((char *)(&pack.time),
                                        sizeof(pack.time))) {
        return pack;
      }
    }

    return std::variant<messenger::network_packets::paxos_notif_packet,
                        messenger::network_packets::paxos_push_packet,
                        network_packets::dialog_text,
                        messenger::network_packets::request_chat_hash>();
  }

  static std::pair<std::shared_ptr<char[]>, uint64_t>
  serialize(const messenger::network_packets::paxos_notif_packet &pack) {
    uint64_t len =
        1 + sizeof(messenger::network_packets::paxos_notif_packet::chat_id) +
        sizeof(messenger::network_packets::paxos_notif_packet::id) +
        sizeof(messenger::network_packets::paxos_notif_packet::hash);
    std::shared_ptr<char[]> data(new char[len]);
    data[0] = network_type::paxos_notif;
    std::memcpy(&data[1], pack.chat_id, sizeof(pack.chat_id));
    std::memcpy(&data[1 + sizeof(pack.chat_id)], pack.id, sizeof(pack.id));
    std::memcpy(&data[1 + sizeof(pack.chat_id) + sizeof(pack.id)], pack.hash,
                sizeof(pack.hash));
    return {data, len};
  }

  static std::pair<std::shared_ptr<char[]>, uint64_t>
  serialize(const messenger::network_packets::paxos_push_packet &obj) {
    uint64_t len = 0;
    std::shared_ptr<char[]> data;

    if (obj.c_event->event_type == chat_event_types::chat_text_type) {
      std::shared_ptr<messenger::chat_text> c_event =
          std::dynamic_pointer_cast<messenger::chat_text>(obj.c_event);
      len = static_cast<uint64_t>(1) + 1 + sizeof(obj.chat_id) +
            sizeof(obj.id) + sizeof(obj.time) +
            c_event->text.size(); // network type+event type+event data
    }
    if (obj.c_event->event_type == chat_event_types::chat_new_user_type) {
      std::shared_ptr<messenger::chat_new_user> c_event =
          std::dynamic_pointer_cast<messenger::chat_new_user>(obj.c_event);
      len = static_cast<uint64_t>(1) + 1 + sizeof(obj.chat_id) +
            sizeof(obj.id) + sizeof(obj.time) + c_event->new_user_id.size(); //
    }

    if (obj.c_event->event_type == chat_event_types::transfer_type) {
      std::shared_ptr<messenger::transfer> c_event =
          std::dynamic_pointer_cast<messenger::transfer>(obj.c_event);
      len = static_cast<uint64_t>(1) + 1 + sizeof(obj.chat_id) +
            sizeof(obj.id) + sizeof(obj.time) + sizeof(c_event->amount) +
            c_event->recipient.size(); //...+512
    }

    data = std::shared_ptr<char[]>(new char[len]);
    auto raw_data = data.get();
    writer data_writer(raw_data, len);
    char network_type = network_type::paxos_push;
    data_writer.writer_sequentially(&network_type, 1);
    data_writer.writer_sequentially(&obj.c_event->event_type, 1);
    data_writer.writer_sequentially(
        obj.chat_id,
        sizeof(messenger::network_packets::paxos_push_packet::chat_id));
    data_writer.writer_sequentially(
        obj.id, sizeof(messenger::network_packets::paxos_push_packet::id));
    data_writer.writer_sequentially(
        reinterpret_cast<const char *>(&obj.time),
        sizeof(messenger::network_packets::paxos_push_packet::time));
    if (obj.c_event->event_type == chat_event_types::chat_text_type) {
      std::shared_ptr<messenger::chat_text> c_event =
          std::dynamic_pointer_cast<messenger::chat_text>(obj.c_event);
      bool res = data_writer.writer_sequentially(c_event->text.c_str(),
                                                 c_event->text.size());
      if (res) {
        return {data, len};
      }
    }
    if (obj.c_event->event_type == chat_event_types::chat_new_user_type) {
      std::shared_ptr<messenger::chat_new_user> c_event =
          std::dynamic_pointer_cast<messenger::chat_new_user>(obj.c_event);
      bool res = data_writer.writer_sequentially(c_event->new_user_id.c_str(),
                                                 c_event->new_user_id.size());
      if (res) {
        return {data, len};
      }
    }

    if (obj.c_event->event_type == chat_event_types::transfer_type) {
      std::shared_ptr<messenger::transfer> c_event =
          std::dynamic_pointer_cast<messenger::transfer>(obj.c_event);
      data_writer.writer_sequentially(
          reinterpret_cast<char *>(&c_event->amount), sizeof(c_event->amount));
      bool res = data_writer.writer_sequentially(c_event->recipient.c_str(),
                                                 c_event->recipient.size());
      if (res) {
        return {data, len};
      }
    }

    return {std::shared_ptr<char[]>(nullptr), 0};
  }

  static std::pair<std::shared_ptr<char[]>, uint64_t>
  serialize(const messenger::network_packets::request_chat_hash &pack) {
    uint64_t len =
        1 + sizeof(pack.chat_id) + sizeof(pack.id) + sizeof(pack.time);
    std::shared_ptr<char[]> data(new char[len]);
    writer data_writer(data.get(), len);
    char type = messenger::network_type::chat_sync;
    data_writer.writer_sequentially(&type, 1);
    data_writer.writer_sequentially(pack.chat_id, sizeof(pack.chat_id));
    data_writer.writer_sequentially(pack.id, sizeof(pack.id));
    bool res = data_writer.writer_sequentially((char *)(&pack.time),
                                               sizeof(pack.time));
    if (res) {
      return {data, len};
    } else {
      return {nullptr, 0};
    }
  }

  static std::pair<std::shared_ptr<char[]>, uint64_t>
  serialize(const messenger::network_packets::dialog_text &pack) {

    auto raw_key = digital_signature::rsa_key_to_bytes(pack.id);
    uint64_t len = 1 + sizeof(raw_key.second) + sizeof(uint64_t) +
                   raw_key.second + pack.text.size();
    std::shared_ptr<char[]> data(new char[len]);
    writer data_writer(data.get(), len);
    char type = messenger::network_type::dialog_text;
    data_writer.writer_sequentially(&type, 1);
    data_writer.writer_sequentially((char *)&raw_key.second,
                                    sizeof(raw_key.second));
    uint64_t text_len = pack.text.size();
    data_writer.writer_sequentially((char *)&text_len, sizeof(text_len));
    data_writer.writer_sequentially((char *)raw_key.first, raw_key.second);
    bool res = data_writer.writer_sequentially(pack.text.c_str(), text_len);
    delete[] raw_key.first;
    if (res) {
      return {data, len};
    } else {
      return {nullptr, 0};
    }
  }
};

} // namespace messenger

#endif // ! DESERIALIZER_H