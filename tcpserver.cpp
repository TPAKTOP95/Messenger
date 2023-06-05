#include "tcpserver.h"
#include "network.h"
#include "network_types.h"

void messenger::network::messenger_server::do_accept() {

  acceptor_.async_accept([this](boost::system::error_code ec,
                                boost::asio::ip::tcp::socket socket) {
    if (!ec) {

      std::shared_ptr<ip::tcp::socket> sock(
          std::make_shared<ip::tcp::socket>(std::move(socket)));
      std::shared_ptr<char> msg_type(new char);
      boost::asio::async_read(
          *sock, boost::asio::buffer(msg_type.get(), 1),
          [msg_type, this, sock](boost::system::error_code ec, uint64_t) {
            if (*msg_type == messenger::network_type::dialog_text) {
              accept_dialog_msg(
                  sock, this->keys,
                  [this](boost::system::error_code ec,
                         std::variant<network_packets::dialog_text> res) {
                    if (std::get_if<network_packets::dialog_text>(&res) !=
                        nullptr) {
                      this->dialog_text_handler(
                          *std::get_if<network_packets::dialog_text>(&res));
                    }
                  });
            }
          });
    } else {
      std::cout << ec.message() << '\n';
    }
    do_accept();
  });
}

void messenger::network::handle_paxos_notif(
    messenger_server *serv,
    std::shared_ptr<network_packets::paxos_notif_packet> pack) {
  std::unique_lock ul{serv->get_paxos_list().second};
  auto it = serv->get_paxos_list().first.find(pack->chat_id);
  if (it != serv->get_paxos_list().first.end()) {
    it->second.accept_promise(pack->id, pack->hash);
  }
}

void messenger::network::handle_paxos_push(
    messenger_server *serv,
    std::shared_ptr<network_packets::paxos_push_packet> pack) {
  std::scoped_lock sl{serv->get_chat_list().second,
                      serv->get_paxos_list().second};
  auto chat_it = serv->get_chat_list().first.find(pack->chat_id);
  auto paxos_it = serv->get_paxos_list().first.find(pack->chat_id);
  if ((chat_it != serv->get_chat_list().first.end()) &&
      (paxos_it != serv->get_paxos_list().first.end())) {
    auto chat_instance = &chat_it->second;
    auto paxos_instance = &paxos_it->second;
    uint64_t current_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();
    auto &c_event = pack->c_event;
    if (c_event->time >= current_time) {

      std::shared_ptr<boost::asio::steady_timer> paxos_start_timer(
          new boost::asio::steady_timer(
              serv->get_io(),
              boost::asio::chrono::milliseconds(c_event->time - current_time)));

      paxos_start_timer->async_wait([&paxos_instance, c_event, &chat_instance,
                                     serv, paxos_start_timer](
                                        const boost::system::error_code &ec) {
        if (paxos_instance->correct_action(c_event)) {
          // send notif
          messenger::network_packets::paxos_notif_packet pack;
          std::memcpy(pack.chat_id, chat_instance->chat_id().c_str(),
                      chat_instance->chat_id().size());
          std::memcpy(pack.id, chat_instance->get_my_id().c_str(),
                      chat_instance->get_my_id().size());
          std::memcpy(pack.hash, chat_instance->new_hash(c_event).c_str(),
                      chat_instance->new_hash(c_event).size());
          auto ser_res = deserializer::serialize(pack);
          auto data = ser_res.first;
          auto len = ser_res.second;
          std::shared_ptr<char[]> data_with_len(new char[len + sizeof(len)]);
          std::memcpy(data_with_len.get(), &len, sizeof(len));
          std::memcpy(data_with_len.get() + sizeof(len), data.get(), len);
          paxos_instance->loop_through_users(
              [serv, data_with_len, len = len + sizeof(len)](
                  std::pair<const std::string, uint32_t> &participant) {
                serv->async_send(data_with_len, len, participant.first,
                                 [](const boost::system::error_code ec) {});
              });
          paxos_instance->start_accept(c_event, 1000);
        }
      });
    }
  }
}

void messenger::network::handle_dialog_text(
    messenger_server *serv,
    std::shared_ptr<network_packets::dialog_text> pack) {}

void messenger::network::request_chat(messenger_server *serv, std::string id,
                                      std::string chat_id) {
  serv->async_connect(id, [serv, chat_id](boost::asio::ip::tcp::socket sock,
                                          const boost::system::error_code ec) {
    std::unique_lock ul{serv->get_chat_list().second};
    auto chat_inst_it = serv->get_chat_list().first.find(chat_id);
    if (chat_inst_it != serv->get_chat_list().first.end()) {
      auto chat_inst = &chat_inst_it->second;
      messenger::network_packets::request_chat_hash pack;
      std::memcpy(pack.chat_id, chat_id.c_str(), chat_id.size());
      std::memcpy(pack.id, chat_inst->get_my_id().c_str(),
                  chat_inst->get_my_id().size());
      pack.time = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();
      auto data = messenger::deserializer::serialize(pack);
      uint64_t len = data.second;
      std::shared_ptr<char[]> data_with_len(new char[len + sizeof(len)]);
      std::memcpy(data_with_len.get(), &len, sizeof(len));
      std::memcpy(data_with_len.get() + sizeof(len), data.first.get(), len);
      std::shared_ptr<ip::tcp::socket> ptr(
          std::make_shared<ip::tcp::socket>(std::move(sock)));
      boost::asio::async_write(
          *ptr, boost::asio::buffer(data_with_len.get(), len + sizeof(len)),
          [data_with_len, chat_inst, ptr](const boost::system::error_code ec,
                                          uint64_t) {
            if (!ec) {
              chat_inst->clear();
              messenger::network::handle_chat_sync_event(chat_inst, ptr);
            }
          });
    }
  });
}

void messenger::network::handle_chat_sync_event(
    messenger::chat *chat_inst, std::shared_ptr<ip::tcp::socket> ptr) {
  std::shared_ptr<uint64_t> len(new uint64_t(0));
  boost::asio::async_read(
      *ptr, boost::asio::buffer(len.get(), sizeof(*len)),
      [ptr, len, chat_inst](const boost::system::error_code ec, uint64_t) {
        if (!ec) {
          std::shared_ptr<char[]> data(new char[*len]);
          boost::asio::async_read(
              *ptr, boost::asio::buffer(data.get(), *len),
              [data, len, chat_inst, ptr](const boost::system::error_code ec,
                                          uint64_t) {
                if (!ec) {
                  auto res_v = deserializer::deserialize(data, *len);
                  auto push_res =
                      std::get_if<network_packets::paxos_push_packet>(&res_v);

                  if (push_res != nullptr) {
                    std::shared_ptr<network_packets::paxos_push_packet> pptr(
                        new network_packets::paxos_push_packet);
                    *pptr = *push_res;
                    chat_inst->add(pptr->c_event);
                  }

                  messenger::network::handle_chat_sync_event(chat_inst, ptr);
                }
              });
        }
      });
}

void messenger::network::share_chat_history(
    messenger_server *serv,
    std::shared_ptr<network_packets::request_chat_hash> pack,
    std::shared_ptr<boost::asio::ip::tcp::socket> ptr, uint64_t ind = 0) {
  std::unique_lock ul{serv->get_chat_list().second};
  auto chat_it = serv->get_chat_list().first.find(pack->chat_id);
  auto chat_inst = &chat_it->second;
  auto el = chat_inst->get(ind);
  if (el != nullptr) {
    std::pair<std::shared_ptr<char[]>, uint64_t> ser_res;
    if (el->event_type == messenger::chat_event_types::chat_text_type) {
      messenger::network_packets::paxos_push_packet evpack;
      std::memcpy(evpack.chat_id, chat_inst->chat_id().c_str(),
                  chat_inst->chat_id().size());
      std::memcpy(evpack.id, chat_inst->get_my_id().c_str(),
                  chat_inst->get_my_id().size());
      evpack.c_event = el;
      evpack.time = 0;
      auto data = messenger::deserializer::serialize(evpack);
      uint64_t len = data.second;
      std::shared_ptr<char[]> data_with_len(new char[len + sizeof(len)]);
      std::memcpy(data_with_len.get(), &len, sizeof(len));
      std::memcpy(data_with_len.get() + sizeof(len), data.first.get(), len);
      boost::asio::async_write(
          *ptr, boost::asio::buffer(data_with_len.get(), len + sizeof(len)),
          [ptr, data_with_len, ind, evpack, serv,
           pack](const boost::system::error_code ec, uint64_t) {
            share_chat_history(serv, pack, ptr, ind + 1);
          });
    }
  }
}