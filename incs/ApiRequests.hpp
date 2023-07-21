#ifndef APIREQUESTS_HPP
#define APIREQUESTS_HPP

#include <string>

#include "crow.h"

struct env {
    std::string user;
    std::string pass;

    env() {
        std::ifstream f;  // TODO : replace with getenv()
        f.open(".env");
        f >> user >> pass;
        f.close();
    }
};

crow::response signUp(const crow::json::rvalue& json);
crow::response signIn(const crow::json::rvalue& json);

#endif  // APIREQUESTS_HPP