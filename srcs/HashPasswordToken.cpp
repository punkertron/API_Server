#include "HashPasswordToken.hpp"

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>

std::string generateHash(const std::string& str) {
    CryptoPP::SHA256 hash;
    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    std::string hashString;

    hash.CalculateDigest(digest,
                         reinterpret_cast<const CryptoPP::byte*>(str.c_str()),
                         str.size());

    CryptoPP::HexEncoder encoder;
    encoder.Attach(new CryptoPP::StringSink(hashString));
    encoder.Put(digest, sizeof(digest));
    encoder.MessageEnd();

    return hashString;
}

bool verifyPassword(const std::string& password,
                    const std::string& storedHash) {
    return generateHash(password) == storedHash;
}

std::string generateToken(void) {
    std::srand(std::time(NULL));

    std::string token;

    const int tokenSize = 64;
    const char* chars =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.";

    for (int i = 0; i < tokenSize; ++i)
        token += chars[std::rand() % std::strlen(chars)];
    return token;
}
