#ifndef AUTHORIZATION_HPP
#define AUTHORIZATION_HPP

#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <mysql_error.h>

#include <iostream>
#include <string>

#include "ApiRequests.hpp"  // REMOVE WHEN ADD getenv
#include "HashPasswordToken.hpp"
#include "crow.h"

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
        sql::mysql::MySQL_Driver* driver =
            sql::mysql::get_mysql_driver_instance();
        sql::Connection* conn =
            driver->connect("tcp://127.0.0.1:3306", e.user, e.pass);

        std::string query =
            "SELECT hash_token FROM server.users WHERE hash_token = ?;";
        sql::PreparedStatement* pstmt = conn->prepareStatement(query);
        pstmt->setString(1, input_hash_token);
        sql::ResultSet* result = pstmt->executeQuery();
        if (result->next())
            res.code = crow::status::ACCEPTED;
        else {
            res.code = crow::status::UNAUTHORIZED;
            res.end();
        }

        conn->close();
        delete conn;
        delete pstmt;
        delete result;

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