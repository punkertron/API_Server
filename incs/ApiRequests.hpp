#ifndef APIREQUESTS_HPP
#define APIREQUESTS_HPP

#include <string>

#include "crow.h"

struct env {
    std::string dbname;
    std::string user;
    std::string password;

    std::string conn_string;

    env() {
        std::ifstream f;  // TODO : replace with getenv()
        f.open(".env");
        f >> dbname >> user >> password;
        f.close();

        conn_string = std::string("dbname=") + dbname + " user=" + user +
                      " password=" + password;
    }
};

crow::response signUp(const crow::json::rvalue& json);
crow::response signIn(const crow::json::rvalue& json);
crow::response uploadFiles(const crow::request& req);

#endif  // APIREQUESTS_HPP