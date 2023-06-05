#ifndef CRYPTO_UTILS_H_
#define CRYPTO_UTILS_H_

#include "cryptopp/cryptlib.h"
#include "cryptopp/dsa.h"
#include "cryptopp/files.h"
#include "cryptopp/filters.h"
#include "cryptopp/hex.h"
#include "cryptopp/osrng.h"
#include "cryptopp/pssr.h"
#include "cryptopp/rsa.h"
#include "cryptopp/salsa.h"
#include "cryptopp/secblock.h"
#include "cryptopp/sha.h"
#include <iostream>
#include <string>

// using namespace CryptoPP;

namespace salsa20 {
struct cipher_info {
  std::shared_ptr<unsigned char[]> cipher;
  uint64_t cipher_size;
  CryptoPP::SecByteBlock key;
  CryptoPP::SecByteBlock iv;
};

CryptoPP::SecByteBlock string_to_secblock(std::string key_str);

std::string secblock_to_string(CryptoPP::SecByteBlock key);

cipher_info salsa20_encrypt(CryptoPP::SecByteBlock iv,
                            CryptoPP::SecByteBlock key, unsigned char *data,
                            uint64_t data_size);

std::pair<std::shared_ptr<unsigned char[]>, int64_t>
salsa20_decrypt(CryptoPP::SecByteBlock iv, CryptoPP::SecByteBlock key,
                const char *cipher, uint64_t cipher_size);

struct sign_obj {
  std::shared_ptr<CryptoPP::byte[]> public_key;
  CryptoPP::lword public_key_size = 0;
  std::shared_ptr<CryptoPP::byte[]> data;
  CryptoPP::lword data_size = 0;
  std::string hash;
};

} // namespace salsa20
namespace digital_signature {

salsa20::sign_obj sign(std::shared_ptr<unsigned char[]> data,
                       uint64_t data_size, CryptoPP::RSA::PrivateKey privateKey,
                       CryptoPP::RSA::PublicKey publicKey);
bool verify(unsigned char *data, uint64_t data_size,
            CryptoPP::RSA::PublicKey publicKey,
            CryptoPP::SecByteBlock signature);

template <class T> std::pair<unsigned char *, int64_t> rsa_key_to_bytes(T key) {
  CryptoPP::ByteQueue bq;
  key.Save(bq);
  CryptoPP::lword key_size = bq.TotalBytesRetrievable();
  unsigned char *key_buf = new unsigned char[key_size];
  bq.Get(key_buf, key_size);
  return {key_buf, key_size};
}

template <class T>
auto bytes_to_rsa_key(const unsigned char *key, int64_t size) {
  CryptoPP::ByteQueue bq;
  bq.Put(key, size);
  T rsa_key;
  rsa_key.Load(bq);
  return rsa_key;
}

template <class T> bool compare_keys(const T &key1, const T &key2) {
  std::string s1 = ::key_exchange::key_to_hex(key1);
  std::string s2 = ::key_exchange::key_to_hex(key2);
  return s1 == s2;
}

} // namespace digital_signature

namespace key_exchange {

void save_private_key(CryptoPP::RSA::PrivateKey &private_key,
                      std::string filename);

void save_public_key(CryptoPP::RSA::PublicKey &public_key,
                     std::string filename);

void load(const std::string &filename, CryptoPP::BufferedTransformation &bt);

void load_private_key(CryptoPP::RSA::PrivateKey &private_key,
                      std::string filename);

void load_public_key(CryptoPP::RSA::PublicKey &public_key,
                     std::string filename);

template <class T>
T bytes_to_rsa_key(
    std::pair<std::shared_ptr<unsigned char[]>, int64_t> key_and_size) {
  CryptoPP::ByteQueue bq;
  bq.Put(key_and_size.first.get(), key_and_size.second);
  T rsa_key;
  rsa_key.Load(bq);
  return rsa_key;
}

template <class T>
std::pair<std::shared_ptr<unsigned char[]>, int64_t> rsa_key_to_bytes(T key) {
  CryptoPP::ByteQueue bq;
  key.Save(bq);
  CryptoPP::lword key_size = bq.TotalBytesRetrievable();
  std::shared_ptr<unsigned char[]> key_buf(new unsigned char[key_size]);
  bq.Get(key_buf.get(), key_size);
  return {key_buf, key_size};
}

template <class T> std::string key_to_hex(T key) {
  auto p = rsa_key_to_bytes(key);
  std::string key_str;
  CryptoPP::HexEncoder encoder;
  encoder.Put(p.first.get(), p.second);
  encoder.MessageEnd();
  CryptoPP::word64 size = encoder.MaxRetrievable();
  if (size) {
    key_str.resize(size);
    encoder.Get((CryptoPP::byte *)&key_str[0], key_str.size());
    return key_str;
  } else {
    throw std::runtime_error("error: key to string conversion");
  }
}

template <class T> T hex_to_key(std::string hex_key) {
  CryptoPP::HexDecoder decoder;
  decoder.Put((CryptoPP::byte *)hex_key.data(), hex_key.size());
  decoder.MessageEnd();
  CryptoPP::word64 size = decoder.MaxRetrievable();
  if (size && size <= SIZE_MAX) {
    std::shared_ptr<unsigned char[]> key(new unsigned char[size]);
    decoder.Get((CryptoPP::byte *)&key[0], size);
    return bytes_to_rsa_key<T>({key, size});
  } else {
    throw std::runtime_error("error: string to key conversion");
  }
}

static CryptoPP::AutoSeededRandomPool prng;

std::pair<CryptoPP::RSA::PublicKey, CryptoPP::RSA::PrivateKey> generate_key();

CryptoPP::Integer rsa_encrypt(unsigned char *data, uint64_t data_size,
                              CryptoPP::RSA::PublicKey public_key);

std::pair<std::shared_ptr<unsigned char[]>, int64_t>
rsa_decrypt(CryptoPP::Integer cipher, CryptoPP::RSA::PrivateKey private_key);

} // namespace key_exchange
#endif