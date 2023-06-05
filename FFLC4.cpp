#include "boost/asio.hpp"
#include "deserializer.h"
#include "mainwindow.h"
#include "network.h"
#include "network_types.h"
#include "paxos.h"
#include "tcpserver.h"
#include "thread_safe_structures.h"
#include <QApplication>
#include <iostream>

int main(int argc, char *argv[]) {
  
  QApplication a(argc, argv);

  thread_safe_map<std::string, std::pair<std::string, char>> address_from_id;
  thread_safe_map<std::string, CryptoPP::RSA::PublicKey> RSA_key_from_id;
  thread_safe_map<std::string, std::string> id_from_rsa_hex;
  std::pair<std::map<std::string, messenger::chat>, std::mutex> chat_list;
  std::pair<std::map<std::string, messenger::paxos>, std::mutex> paxos_list;

  boost::asio::io_context io;
  auto keys = key_exchange::generate_key();
  std::cout << key_exchange::key_to_hex(keys.first) << "<- ";
  CryptoPP::RSA::PrivateKey sign_privateKey = keys.second;
  CryptoPP::RSA::PublicKey sign_publicKey = keys.first;


  messenger::network::messenger_server serv(
      40000, chat_list, paxos_list, address_from_id,
      sign_privateKey,
      sign_publicKey, io);

  MainWindow w;
  w.setWindowTitle("FFLC");
  w.show();

  serv.dialog_text_handler =
      [&w,
       &id_from_rsa_hex](messenger::network_packets::dialog_text id_and_text) {
        id_and_text.id;
        std::string key_str =
            id_from_rsa_hex.get(key_exchange::key_to_hex(id_and_text.id));
        bool already_exist = false;
        for (int i = 0; i < w.get_list()->count(); i++) {
          if (w.get_list()->item(i)->text().toStdString() == key_str) {
            already_exist = true;
            break;
          }
        }
        if (!already_exist) {
          QListWidgetItem *item = new QListWidgetItem(QString(key_str.c_str()));
          w.get_list()->addItem(item);
        }
        w.get_history()[key_str] += id_and_text.text;
        std::cout << key_str << " " << id_and_text.text << std::endl;
        std::string s(key_str);
        int c = 4;
      };

  QObject::connect(
      &w, &MainWindow::button_send,
      [serv = &serv, &RSA_key_from_id](std::string id, std::string text) {
        auto key = RSA_key_from_id.get(id);
        std::string s = key_exchange::key_to_hex(key);

        messenger::network::send_dialog_msg(serv, id, text, serv->keys.first,
                                            serv->keys.second, key,
                                            [](boost::system::error_code ec) {
                                              std::string error(ec.message());
                                              std::cout << ec.message();
                                              std::cout << "error" << std::endl;
                                            });
      });

  QObject::connect(
      &w, &MainWindow::button_close_addition,
      [&address_from_id, &RSA_key_from_id, &id_from_rsa_hex](
          std::string name, std::string public_key, std::string ip_address) {
        std::cout << name << " " << public_key << " " << ip_address
                  << std::endl;
        address_from_id.add(name, std::pair{std::string(ip_address), (char)0});
        RSA_key_from_id.add(
            name,
            key_exchange::hex_to_key<CryptoPP::RSA::PublicKey>(public_key));
        id_from_rsa_hex.add(public_key, name);
      });

  std::thread thr1([&io]() { io.run(); });
  std::thread thr2([&io]() { io.run(); });
  std::thread thr3([&io]() { io.run(); });
  a.exec();
  thr1.join();
  thr2.join();
  thr3.join();
}
