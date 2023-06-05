#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "chat.h"
#include "chat_info_list.h"
#include "deserializer.h"
#include "paxos.h"
#include <boost/asio.hpp>
#include <cryptopp/rsa.h>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace messenger {
namespace network {
using namespace boost::asio;

class messenger_server {
public:
  messenger_server(
      short port,
      std::pair<std::map<std::string, messenger::chat>, std::mutex> &c_list,
      std::pair<std::map<std::string, messenger::paxos>, std::mutex> &p_list,
      thread_safe_map<std::string, std::pair<std::string, char>> &ip_id,
      CryptoPP::RSA::PrivateKey prk, CryptoPP::RSA::PublicKey pbk,
      boost::asio::io_context &io)
      : chat_list(c_list), paxos_list(p_list), resolver(io),
        acceptor_(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                                                     port)),
        io_context(io), ip_from_id(ip_id), keys(pbk, prk) {
    std::cout << acceptor_.local_endpoint() << std::endl;
    do_accept();
  }

  template <typename functor>
  void async_send(std::shared_ptr<char[]> data, uint64_t len,
                  const std::string id, functor handler) {

    std::string host, port;
    std::string add = this->ip_from_id.get(id).first;
    auto pos = add.find(':');
    if (pos != std::string::npos) {
      host = add.substr(0, pos);
      port = add.substr(pos + 1);
    }

    resolver.async_resolve(
        host, port,
        [this, handler, data,
         len](const boost::system::error_code &ec,
              boost::asio::ip::tcp::resolver::results_type results) {
          if (!ec) {
            std::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr =
                std::make_shared<boost::asio::ip::tcp::socket>(
                    this->io_context);
            boost::asio::async_connect(
                *socket_ptr, results,
                [socket_ptr, handler, data,
                 len](const boost::system::error_code &ec,
                      const boost::asio::ip ::tcp::endpoint &) {
                  if (!ec) {
                    boost::asio::async_write(
                        *socket_ptr, boost::asio::buffer(data.get(), len),
                        [handler, socket_ptr,
                         data](const boost::system::error_code &ec,
                               uint64_t bytes) { handler(ec); });
                  } else {
                    handler(ec);
                  }
                });
          } else {
            handler(ec);
          }
        });
  }

  template <typename functor>
  void async_connect(const std::string id, functor handler) {

    std::string host, port;
    std::string add = this->ip_from_id.get(id).first;
    auto pos = add.find(':');
    if (pos != std::string::npos) {
      host = add.substr(0, pos);
      port = add.substr(pos + 1);
    }

    resolver.async_resolve(
        host, port,
        [this, handler](const boost::system::error_code &ec,
                        boost::asio::ip::tcp::resolver::results_type results) {
          if (!ec) {
            std::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr =
                std::make_shared<boost::asio::ip::tcp::socket>(
                    this->io_context);

            boost::asio::async_connect(
                *socket_ptr, results,
                [socket_ptr, handler](const boost::system::error_code &ec,
                                      const boost::asio::ip ::tcp::endpoint &) {
                  handler(std::move(*socket_ptr), ec);
                });
          } else {
            handler(boost::asio::ip::tcp::socket(this->get_io()), ec);
          }
        });
  }

  auto &get_chat_list() { return chat_list; }
  auto &get_paxos_list() { return paxos_list; }

  auto &get_io() { return io_context; }
  auto &get_ip_from_id() { return ip_from_id; }

  std::pair<CryptoPP::RSA::PublicKey, CryptoPP::RSA::PrivateKey> keys;
  std::function<void(messenger::network_packets::dialog_text)>
      dialog_text_handler;

private:
  void do_accept();

  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::io_context &io_context;
  boost::asio::ip::tcp::resolver resolver;
  std::pair<std::map<std::string, messenger::chat>, std::mutex> &chat_list;
  std::pair<std::map<std::string, messenger::paxos>, std::mutex> &paxos_list;
  thread_safe_map<std::string, std::pair<std::string, char>> &ip_from_id;
};

void handle_paxos_notif(
    messenger_server *serv,
    std::shared_ptr<network_packets::paxos_notif_packet> pack);

void handle_paxos_push(
    messenger_server *serv,
    std::shared_ptr<network_packets::paxos_push_packet> pack);

void handle_dialog_text(messenger_server *serv,
                        std::shared_ptr<network_packets::dialog_text> pack);

void request_chat(messenger_server *serv, std::string id, std::string chat_id);

void handle_chat_sync_event(messenger::chat *chat_inst,
                            std::shared_ptr<ip::tcp::socket> ptr);

void share_chat_history(
    messenger_server *serv,
    std::shared_ptr<network_packets::request_chat_hash> pack,
    std::shared_ptr<boost::asio::ip::tcp::socket> ptr, uint64_t ind);
} // namespace network

} // namespace messenger

#endif
