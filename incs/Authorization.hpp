#ifndef AUTHORIZATION_HPP
#define AUTHORIZATION_HPP

#include <cryptopp/hex.h>
#include <cryptopp/sha.h>

#include <iostream>
#include <string>

#include "crow.h"

static std::map<std::string, std::string> users = {
    {"user1", "password1"},
    {"user2", "password2"},
};

struct Authorization : crow::ILocalMiddleware {
    struct context {};

    void before_handle(crow::request& req, crow::response& res, context& ctx) {
        if (req.headers.count("Authorization") == 0) {
            res.code = 401;  // Unauthorized
            res.end();
            return;
        }
        std::string auth(req.get_header_value("Authorization"));
        if (auth.substr(0, 6) != "Basic ") {
            res.code = 401;
            res.end();
            return;
        }
        std::string credentials(auth.substr(6));
        std::size_t colonPos = credentials.find(':');
        if (colonPos == std::string::npos) {
            res.code = 401;
            res.end();
            return;
        }

        std::string username = credentials.substr(0, colonPos);
        std::string password = credentials.substr(colonPos + 1);

        auto it = users.find(username);
        if (it == users.end() || it->second != password) {
            res.code = crow::status::UNAUTHORIZED;
            res.end();
            return;
        }
        (void)ctx;
    }

    void after_handle(crow::request& req, crow::response& res, context& ctx) {
        (void)req;
        (void)res;
        (void)ctx;
    }
};

#endif  // AUTHORIZATION_HPP