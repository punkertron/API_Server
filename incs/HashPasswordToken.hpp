#ifndef HASHPASSWORD_HPP
#define HASHPASSWORD_HPP

#include <cryptopp/hex.h>
#include <cryptopp/sha.h>

#include <string>

std::string generateHash(const std::string& str);
bool verifyPassword(const std::string& password, const std::string& storedHash);
std::string generateToken(void);

#endif