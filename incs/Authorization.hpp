#ifndef AUTHORIZATION_HPP
#define AUTHORIZATION_HPP

#include <iostream>
#include <pqxx/pqxx>
#include <string>

#include "crow_all.h"

#include "ApiRequests.hpp"  // REMOVE WHEN ADD getenv
#include "HashPasswordToken.hpp"

struct Authorization : crow::ILocalMiddleware {
    struct context {};

    void before_handle(crow::request& req, crow::response& res, context& ctx) {
        std::string auth;

        if (req.headers.count("Authorization") == 0 ||
            (auth = req.get_header_value("Authorization")).substr(0, 6) !=
                "Bearer") {
            res.code = crow::status::UNAUTHORIZED;
            res.end();
            return;
        }

        std::string input_hash_token(generateHash(auth.substr(7)));

        env e;
        pqxx::connection c(e.conn_string);
        pqxx::work w(c);

        c.prepare("find",
                  "SELECT hash_token FROM server.users WHERE hash_token = $1");
        pqxx::result result = w.exec_prepared("find", input_hash_token);

        if (result.size())
            res.code = crow::status::ACCEPTED;
        else {
            res.code = crow::status::UNAUTHORIZED;
            res.end();
        }
        c.close();

        (void)ctx;
        return;
    }

    void after_handle(crow::request& req, crow::response& res, context& ctx) {
        (void)req;
        (void)res;
        (void)ctx;
    }
};

#endif  // AUTHORIZATION_HPP