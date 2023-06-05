#include "crypto_utils.h"

namespace salsa20 {
CryptoPP::SecByteBlock string_to_secblock(std::string key_str) {
  return {(const CryptoPP::byte *)(key_str.data()), key_str.size()};
}

std::string secblock_to_string(CryptoPP::SecByteBlock key) {
  return {(const char *)key.data(), key.size()};
}

cipher_info salsa20_encrypt(CryptoPP::SecByteBlock iv,
                            CryptoPP::SecByteBlock key, unsigned char *data,
                            uint64_t data_size) {

  CryptoPP::AutoSeededRandomPool prng;
  // Encryption object
  CryptoPP::Salsa20::Encryption enc;
  enc.SetKeyWithIV(key, key.size(), iv, iv.size());

  // Perform the encryption
  std::shared_ptr<unsigned char[]> cipher(new unsigned char[data_size]);
  enc.ProcessData((CryptoPP::byte *)&cipher[0], (const CryptoPP::byte *)data,
                  data_size);

  return cipher_info{cipher, data_size, key, iv};
}

std::pair<std::shared_ptr<unsigned char[]>, int64_t>
salsa20_decrypt(CryptoPP::SecByteBlock iv, CryptoPP::SecByteBlock key,
                const char *cipher, uint64_t cipher_size) {
  std::shared_ptr<unsigned char[]> recover(new unsigned char[cipher_size]);
  CryptoPP::Salsa20::Decryption dec;
  dec.SetKeyWithIV(key, key.size(), iv, iv.size());
  dec.ProcessData((CryptoPP::byte *)&recover[0], (const CryptoPP::byte *)cipher,
                  cipher_size);
  return {recover, cipher_size};
}

} // namespace salsa20

namespace key_exchange {
void save_private_key(CryptoPP::RSA::PrivateKey &private_key,
                      std::string filename) {
  CryptoPP::FileSink output(filename.c_str());
  private_key.DEREncode(output);
}

void save_public_key(CryptoPP::RSA::PublicKey &public_key,
                     std::string filename) {
  CryptoPP::FileSink output(filename.c_str());
  public_key.DEREncode(output);
}

void load(const std::string &filename, CryptoPP::BufferedTransformation &bt) {
  CryptoPP::FileSource file(filename.c_str(), true);
  file.TransferTo(bt);
  bt.MessageEnd();
}

void load_private_key(CryptoPP::RSA::PrivateKey &private_key,
                      std::string filename) {
  CryptoPP::ByteQueue queue;
  load(filename, queue);
  private_key.Load(queue);
}

void load_public_key(CryptoPP::RSA::PublicKey &public_key,
                     std::string filename) {
  CryptoPP::ByteQueue queue;
  load(filename, queue);
  public_key.Load(queue);
}

std::pair<CryptoPP::RSA::PublicKey, CryptoPP::RSA::PrivateKey> generate_key() {
  CryptoPP::InvertibleRSAFunction params;
  params.GenerateRandomWithKeySize(key_exchange::prng, 3072);
  CryptoPP::RSA::PrivateKey private_key(params);
  CryptoPP::RSA::PublicKey public_key(params);
  return std::make_pair(public_key, private_key);
}

CryptoPP::Integer rsa_encrypt(unsigned char *data, uint64_t data_size,
                              CryptoPP::RSA::PublicKey public_key) {
  CryptoPP::Integer hex_text =
      CryptoPP::Integer((const CryptoPP::byte *)data, data_size);
  CryptoPP::Integer encrypted = public_key.ApplyFunction(hex_text);
  return encrypted;
}

std::pair<std::shared_ptr<unsigned char[]>, int64_t>
rsa_decrypt(CryptoPP::Integer cipher, CryptoPP::RSA::PrivateKey private_key) {
  std::string recovered;
  CryptoPP::Integer r = private_key.CalculateInverse(prng, cipher);
  std::size_t rec_size = r.MinEncodedSize();
  std::shared_ptr<unsigned char[]> rec(new unsigned char[rec_size]);
  r.Encode((CryptoPP::byte *)rec.get(), rec_size);
  return {rec, rec_size};
}

} // namespace key_exchange

namespace digital_signature {
salsa20::sign_obj sign(std::shared_ptr<unsigned char[]> data,
                       uint64_t data_size, CryptoPP::RSA::PrivateKey privateKey,
                       CryptoPP::RSA::PublicKey publicKey) {
  CryptoPP::AutoSeededRandomPool sign_rng;
  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA256>::Signer signer(privateKey);
  size_t length = signer.MaxSignatureLength();
  CryptoPP::SecByteBlock signature(length);
  length = signer.SignMessage(sign_rng, (const CryptoPP::byte *)(data.get()),
                              data_size, signature);
  signature.resize(length);

  CryptoPP::ByteQueue bq;
  publicKey.Save(bq);
  CryptoPP::lword pub_key_size = bq.TotalBytesRetrievable();
  std::shared_ptr<CryptoPP::byte[]> pub_key_buf(
      new CryptoPP::byte[pub_key_size]);
  bq.Get(pub_key_buf.get(), pub_key_size);

  salsa20::sign_obj s;
  s.public_key = pub_key_buf;
  s.public_key_size = pub_key_size;
  s.data = data;
  s.data_size = data_size;
  s.hash = salsa20::secblock_to_string(signature);
  return s;
}

bool verify(unsigned char *data, uint64_t data_size,
            CryptoPP::RSA::PublicKey publicKey,
            CryptoPP::SecByteBlock signature) {
  CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA256>::Verifier verifier(
      publicKey);
  bool result = verifier.VerifyMessage((const CryptoPP::byte *)data, data_size,
                                       signature, signature.size());

  return result;
}
} // namespace digital_signature
