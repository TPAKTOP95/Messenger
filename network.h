#ifndef NETWORK_H
#define NETWORK_H
#include "boost/asio.hpp"
#include "crypto_utils.h"
#include "deserializer.h"
#include <string>

namespace messenger {
namespace network {

template <typename serv_type, typename functor>
void send_dialog_msg(serv_type *serv, std::string id, std::string text,
                     CryptoPP::RSA::PublicKey sign_publicKey,
                     CryptoPP::RSA::PrivateKey sign_privateKey,
                     CryptoPP::RSA::PublicKey recipient_public_sign_key,
                     functor handler) {

  serv->async_connect(id, [=](boost::asio::ip::tcp::socket conn_sock,
                              boost::system::error_code ec) {
    std::shared_ptr<boost::asio::ip::tcp::socket> sock(
        std::make_shared<boost::asio::ip::tcp::socket>(std::move(conn_sock)));
    if (ec) {
      handler(ec);
      return;
    }

    auto rsa_key = key_exchange::generate_key();
    auto rsa_key_for_send = rsa_key.first;

    CryptoPP::ByteQueue queue;
    rsa_key_for_send.Save(queue);
    CryptoPP::lword size = queue.TotalBytesRetrievable();
    std::shared_ptr<CryptoPP::byte[]> buf(new CryptoPP::byte[size]);
    queue.Get(buf.get(), size);

    salsa20::sign_obj signed_data =
        digital_signature::sign(std::static_pointer_cast<unsigned char[]>(buf),
                                size, sign_privateKey, sign_publicKey);

    uint64_t key_len = signed_data.public_key_size;
    uint64_t hash_size = signed_data.hash.size();
    uint64_t data_len = signed_data.data_size;
    uint64_t total_len = 1 + sizeof(key_len) + sizeof(hash_size) +
                         sizeof(data_len) + key_len + hash_size + data_len;
    std::shared_ptr<char[]> data(new char[total_len]);
    messenger::writer data_writer(data.get(), total_len);
    char msg_type = messenger::network_type::dialog_text;
    data_writer.writer_sequentially(&msg_type, sizeof(msg_type));
    data_writer.writer_sequentially((char *)(&key_len), sizeof(key_len));
    data_writer.writer_sequentially((char *)(&hash_size), sizeof(hash_size));
    data_writer.writer_sequentially((char *)(&data_len), sizeof(data_len));
    data_writer.writer_sequentially((char *)signed_data.public_key.get(),
                                    key_len);
    data_writer.writer_sequentially(signed_data.hash.c_str(), hash_size);
    bool res = data_writer.writer_sequentially((char *)signed_data.data.get(),
                                               data_len);
    if (!res) {
      handler(
          boost::system::errc::make_error_code(boost::system::errc::io_error));
      return;
    }
    boost::asio::async_write(
        *sock, boost::asio::buffer(data.get(), total_len),
        [data, handler, sock, rsa_decrypt_key = rsa_key.second, sign_publicKey,
         text,
         recipient_public_sign_key](boost::system::error_code ec, uint64_t) {
          if (!ec) {
            accept_cryped_signed_salsa_key(
                sock, rsa_decrypt_key,
                [handler, sign_publicKey = std::move(sign_publicKey),
                 text = std::move(text), sock, recipient_public_sign_key](
                    boost::system::error_code ec, bool if_valid,
                    CryptoPP::RSA::PublicKey accepted_piblic_key_for_sign,
                    CryptoPP::SecByteBlock salsa_iv,
                    CryptoPP::SecByteBlock salsa_key) {
                  if (!ec) {
                    if (digital_signature::compare_keys(
                            accepted_piblic_key_for_sign,
                            recipient_public_sign_key)) {
                      messenger::network_packets::dialog_text pack;
                      pack.id = sign_publicKey;
                      pack.text = std::move(text);
                      auto res = messenger::deserializer::serialize(pack);
                      salsa20::cipher_info crypted_text_pack =
                          salsa20::salsa20_encrypt(
                              salsa_iv, salsa_key,
                              (unsigned char *)res.first.get(), res.second);
                      std::shared_ptr<uint64_t> cipher_size(
                          new uint64_t(crypted_text_pack.cipher_size));
                      std::shared_ptr<std::vector<boost::asio::const_buffer>>
                          buffers(new std::vector<boost::asio::const_buffer>);
                      buffers->push_back(boost::asio::buffer(
                          cipher_size.get(), sizeof(*cipher_size)));
                      buffers->push_back(boost::asio::buffer(
                          crypted_text_pack.cipher.get(), *cipher_size));

                      boost::asio::async_write(
                          *sock, *buffers,
                          [sock, buffers, cipher_size, crypted_text_pack,
                           handler](boost::system::error_code ec, uint64_t) {
                            handler(ec);
                          });
                    } else {
                      handler(boost::system::errc::make_error_code(
                          boost::system::errc::bad_message));
                    }

                  } else {
                    handler(ec);
                  }
                });
          } else {
            handler(ec);
          }
        });
  });
}

template <typename functor>
void accept_cryped_signed_salsa_key(
    std::shared_ptr<boost::asio::ip::tcp::socket> sock,
    CryptoPP::RSA::PrivateKey decrypt_rsa_key, functor handler) {
  std::shared_ptr<char[]> lens(new char[3 * sizeof(uint64_t)]);
  boost::asio::async_read(
      *sock, boost::asio::buffer(lens.get(), 3 * sizeof(uint64_t)),
      [sock, lens, handler, decrypt_rsa_key](boost::system::error_code ec,
                                             uint64_t) {
        if (ec) {
          handler(ec, false, CryptoPP::RSA::PublicKey(),
                  CryptoPP::SecByteBlock(8), CryptoPP::SecByteBlock(16));
        } else {
          uint64_t public_key_len = 0;
          uint64_t hash_len = 0;
          uint64_t data_len = 0;
          std::memcpy(&public_key_len, lens.get(), sizeof(public_key_len));
          std::memcpy(&hash_len, lens.get() + sizeof(public_key_len),
                      sizeof(hash_len));
          std::memcpy(&data_len,
                      lens.get() + sizeof(public_key_len) + sizeof(hash_len),
                      sizeof(data_len));
          std::shared_ptr<char[]> raw_data(
              new char[public_key_len + hash_len + data_len]);
          boost::asio::async_read(
              *sock,
              boost::asio::buffer(raw_data.get(),
                                  public_key_len + hash_len + data_len),
              [sock, public_key_len, hash_len, data_len, raw_data, handler,
               decrypt_rsa_key](boost::system::error_code ec, uint64_t) {
                if (!ec) {
                  salsa20::sign_obj rsa_sign_key;
                  rsa_sign_key.data_size = data_len;
                  rsa_sign_key.public_key_size = public_key_len;
                  rsa_sign_key.data = std::shared_ptr<CryptoPP::byte[]>(
                      new CryptoPP::byte[data_len]);
                  rsa_sign_key.public_key = std::shared_ptr<CryptoPP::byte[]>(
                      new CryptoPP::byte[public_key_len]);
                  std::memcpy(rsa_sign_key.public_key.get(), raw_data.get(),
                              public_key_len);
                  std::unique_ptr<char[]> hash_str(new char[public_key_len]);
                  std::memcpy(hash_str.get(), raw_data.get() + public_key_len,
                              hash_len);
                  rsa_sign_key.hash = std::string(hash_str.get(), hash_len);
                  std::memcpy(rsa_sign_key.data.get(),
                              raw_data.get() + public_key_len + hash_len,
                              data_len);

                  /// //////////////////////////
                  CryptoPP::RSA::PublicKey public_key_for_signature;
                  CryptoPP::SecByteBlock signature =
                      salsa20::string_to_secblock(rsa_sign_key.hash);
                  CryptoPP::ByteQueue bq;
                  bq.Put(rsa_sign_key.public_key.get(),
                         rsa_sign_key.public_key_size);
                  public_key_for_signature.Load(bq);

                  std::string tmp((char *)(rsa_sign_key.data.get()),
                                  rsa_sign_key.data_size);
                  std::stringstream ss_format_data;
                  ss_format_data << tmp;
                  CryptoPP::Integer cipher_info;
                  ss_format_data >> cipher_info;
                  // need to parse and get two secbyteblocks
                  auto salsa_key_and_iv =
                      key_exchange::rsa_decrypt(cipher_info, decrypt_rsa_key);
                  bool res = digital_signature::verify(
                      static_cast<unsigned char *>(rsa_sign_key.data.get()),
                      rsa_sign_key.data_size, public_key_for_signature,
                      signature);
                  if (res) {
                    CryptoPP::SecByteBlock iv(8);
                    CryptoPP::SecByteBlock key(16);
                    std::memcpy(iv, salsa_key_and_iv.first.get(), iv.size());
                    std::memcpy(key, salsa_key_and_iv.first.get() + iv.size(),
                                key.size());
                    handler(boost::system::errc::make_error_code(
                                boost::system::errc::success),
                            res, public_key_for_signature, iv, key);
                  } else {
                    handler(boost::system::errc::make_error_code(
                                boost::system::errc::bad_message),
                            res, public_key_for_signature,
                            CryptoPP::SecByteBlock(8),
                            CryptoPP::SecByteBlock(16));
                  }
                } else {
                  handler(ec, false, CryptoPP::RSA::PublicKey(),
                          CryptoPP::SecByteBlock(8),
                          CryptoPP::SecByteBlock(16));
                }
              });
        }
      });
}

template <typename functor>
void accept_signed_rsa_key(
    std::shared_ptr<boost::asio::ip::tcp::socket> sock,
    functor handler) { // boost::system::error_code ec res, id, rsa_key
  std::shared_ptr<char[]> lens(new char[3 * sizeof(uint64_t)]);
  using namespace boost::asio;
  async_read(
      *sock, boost::asio::buffer(lens.get(), 3 * sizeof(uint64_t)),
      [sock, lens, handler](boost::system::error_code ec, uint64_t) {
        if (!ec) {
          uint64_t public_key_len = 0;
          uint64_t hash_len = 0;
          uint64_t data_len = 0;
          std::memcpy(&public_key_len, lens.get(), sizeof(public_key_len));
          std::memcpy(&hash_len, lens.get() + sizeof(public_key_len),
                      sizeof(hash_len));
          std::memcpy(&data_len,
                      lens.get() + sizeof(public_key_len) + sizeof(hash_len),
                      sizeof(data_len));
          std::shared_ptr<char[]> raw_data(
              new char[public_key_len + hash_len + data_len]);
          async_read(
              *sock,
              boost::asio::buffer(raw_data.get(),
                                  public_key_len + hash_len + data_len),
              [sock, public_key_len, hash_len, data_len, raw_data,
               handler](boost::system::error_code ec, uint64_t) {
                if (!ec) {
                  salsa20::sign_obj rsa_sign_key;
                  rsa_sign_key.data_size = data_len;
                  rsa_sign_key.public_key_size = public_key_len;
                  rsa_sign_key.data = std::shared_ptr<CryptoPP::byte[]>(
                      new CryptoPP::byte[data_len]);
                  rsa_sign_key.public_key = std::shared_ptr<CryptoPP::byte[]>(
                      new CryptoPP::byte[public_key_len]);
                  std::memcpy(rsa_sign_key.public_key.get(), raw_data.get(),
                              public_key_len);
                  std::unique_ptr<char[]> hash_str(new char[public_key_len]);
                  std::memcpy(hash_str.get(), raw_data.get() + public_key_len,
                              hash_len);
                  rsa_sign_key.hash = std::string(hash_str.get(), hash_len);
                  std::memcpy(rsa_sign_key.data.get(),
                              raw_data.get() + public_key_len + hash_len,
                              data_len);

                  CryptoPP::RSA::PublicKey public_key_for_signature;
                  CryptoPP::SecByteBlock signature =
                      salsa20::string_to_secblock(rsa_sign_key.hash);
                  CryptoPP::ByteQueue bq;
                  bq.Put(rsa_sign_key.public_key.get(),
                         rsa_sign_key.public_key_size);
                  public_key_for_signature.Load(bq);
                  //
                  CryptoPP::ByteQueue bq2;
                  bq2.Put(rsa_sign_key.data.get(), rsa_sign_key.data_size);
                  CryptoPP::RSA::PublicKey rsa_public_key;
                  rsa_public_key.Load(bq2);

                  bool res = digital_signature::verify(
                      rsa_sign_key.data.get(), rsa_sign_key.data_size,
                      public_key_for_signature, signature);
                  if (res) {
                    handler(boost::system::errc::make_error_code(
                                boost::system::errc::success),
                            public_key_for_signature, rsa_public_key);
                  } else {
                    handler(boost::system::errc::make_error_code(
                                boost::system::errc::bad_message),
                            CryptoPP::RSA::PublicKey(),
                            CryptoPP::RSA::PublicKey());
                  }

                } else {
                  handler(ec, CryptoPP::RSA::PublicKey(),
                          CryptoPP::RSA::PublicKey());
                }
              });
        } else {
          handler(ec, CryptoPP::RSA::PublicKey(), CryptoPP::RSA::PublicKey());
        }
      });
}

template <typename functor>
void send_crypted_signed_salsa_key(
    std::shared_ptr<boost::asio::ip::tcp::socket> sock,
    CryptoPP::RSA::PublicKey public_sign_key,
    CryptoPP::RSA::PrivateKey private_sign_key,
    CryptoPP::RSA::PublicKey rsa_key, functor handler) {
  CryptoPP::AutoSeededRandomPool prng;
  CryptoPP::SecByteBlock salsa_key(16);
  CryptoPP::SecByteBlock salsa_iv(8);
  prng.GenerateBlock(salsa_iv, salsa_iv.size());
  prng.GenerateBlock(salsa_key, salsa_key.size());

  std::shared_ptr<unsigned char[]> raw_salsa(new unsigned char[24]);
  std::memcpy(raw_salsa.get(), salsa_iv.data(),
              salsa_iv.size()); // must be 16
  std::memcpy(raw_salsa.get() + salsa_iv.size(), salsa_key.data(),
              salsa_key.size());
  auto crypted_salsa_key =
      key_exchange::rsa_encrypt(raw_salsa.get(), 24, rsa_key);
  std::stringstream ss;
  ss << crypted_salsa_key;
  std::string salsa_str = ss.str();
  uint64_t len = salsa_str.size();

  std::shared_ptr<unsigned char[]> raw_str(new unsigned char[len]);
  std::memcpy(raw_str.get(), salsa_str.data(), len);
  auto signed_data =
      digital_signature::sign(raw_str, len, private_sign_key, public_sign_key);

  uint64_t key_len = signed_data.public_key_size;
  uint64_t hash_size = signed_data.hash.size();
  uint64_t data_len = signed_data.data_size;
  uint64_t total_len = sizeof(key_len) + sizeof(hash_size) + sizeof(data_len) +
                       key_len + hash_size + data_len;
  std::shared_ptr<char[]> data(new char[total_len]);
  messenger::writer data_writer(data.get(), total_len);
  data_writer.writer_sequentially((char *)(&key_len), sizeof(key_len));
  data_writer.writer_sequentially((char *)(&hash_size), sizeof(hash_size));
  data_writer.writer_sequentially((char *)(&data_len), sizeof(data_len));
  data_writer.writer_sequentially((char *)signed_data.public_key.get(),
                                  key_len);
  data_writer.writer_sequentially(signed_data.hash.c_str(), hash_size);
  bool res =
      data_writer.writer_sequentially((char *)signed_data.data.get(), data_len);
  if (!res) {
    handler(boost::system::errc::make_error_code(boost::system::errc::io_error),
            salsa_iv, salsa_key);
  } else {
    boost::asio::async_write(*sock, boost::asio::buffer(data.get(), total_len),
                             [sock, data, salsa_iv, salsa_key,
                              handler](boost::system::error_code ec, uint64_t) {
                               handler(ec, salsa_iv, salsa_key);
                             });
  }
}

template <typename functor> // fuctor(error, size, data)
void accept_salsa_crypted_data(
    std::shared_ptr<boost::asio::ip::tcp::socket> sock,
    CryptoPP::SecByteBlock iv, CryptoPP::SecByteBlock key, functor handler) {
  std::shared_ptr<uint64_t> size(new uint64_t(0));
  boost::asio::async_read(
      *sock, boost::asio::buffer(size.get(), sizeof(*size)),
      [size, sock, handler, key = std::move(key),
       iv = std::move(iv)](boost::system::error_code ec, uint64_t) {
        if (!ec) {

          std::shared_ptr<char[]> data(new char[*size]);

          boost::asio::async_read(
              *sock, boost::asio::buffer(data.get(), *size),
              [sock, data, handler, size, key = std::move(key),
               iv = std::move(iv)](boost::system::error_code ec, uint64_t) {
                if (!ec) {
                  CryptoPP::Salsa20::Decryption dec;
                  dec.SetKeyWithIV(key, key.size(), iv, iv.size());
                  std::shared_ptr<char[]> decrypted_data(new char[*size]);
                  dec.ProcessData((unsigned char *)decrypted_data.get(),
                                  (unsigned char *)data.get(), *size);

                  handler(boost::system::errc::make_error_code(
                              boost::system::errc::success),
                          decrypted_data, *size);
                } else {
                  handler(ec, nullptr, 0);
                }
              });

        } else {
          handler(ec, nullptr, 0);
        }
      });
}

template <typename functor>
void accept_dialog_msg(
    std::shared_ptr<boost::asio::ip::tcp::socket> sock,
    std::pair<CryptoPP::RSA::PublicKey, CryptoPP::RSA::PrivateKey> my_sign_keys,
    functor handler) {
  accept_signed_rsa_key(sock, [sock, my_sign_keys,
                               handler](boost::system::error_code ec,
                                        CryptoPP::RSA::PublicKey sender_id,
                                        CryptoPP::RSA::PublicKey rsa_key) {
    if (!ec) {
      send_crypted_signed_salsa_key(
          sock, my_sign_keys.first, my_sign_keys.second, rsa_key,
          [sock, handler, sender_id](boost::system::error_code ec,
                                     CryptoPP::SecByteBlock salsa_iv,
                                     CryptoPP::SecByteBlock salsa_key) {
            if (!ec) {
              accept_salsa_crypted_data(
                  sock, salsa_iv, salsa_key,
                  [sock, handler, sender_id](boost::system::error_code ec,
                                             std::shared_ptr<char[]> data,
                                             uint64_t size) {
                    if (!ec) {
                      auto accepted_text_pack =
                          messenger::deserializer::deserialize(data, size);
                      auto res =
                          std::get_if<messenger::network_packets::dialog_text>(
                              &accepted_text_pack);
                      if (res != nullptr) {
                        if (digital_signature::compare_keys(sender_id,
                                                            res->id)) {
                          handler(
                              ec,
                              std::variant<network_packets::dialog_text>(*res));
                        } else {
                          handler(ec,
                                  std::variant<network_packets::dialog_text>());
                        }
                      }
                    } else {
                      handler(ec, std::variant<network_packets::dialog_text>());
                    }
                  });
            } else {
              handler(ec, std::variant<network_packets::dialog_text>());
            }
          });
    } else {
      handler(ec, std::variant<network_packets::dialog_text>());
    }
  });
}

} // namespace network
} // namespace messenger

#endif