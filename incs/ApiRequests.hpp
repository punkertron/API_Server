#ifndef APIREQUESTS_HPP
#define APIREQUESTS_HPP

#include <string>

#include "crow_all.h"

struct env
{
    std::string dbname;
    std::string user;
    std::string password;

    std::string conn_string;

    env()
    {
        std::ifstream f;  // TODO : replace with getenv()
        f.open(".env");
        f >> dbname >> user >> password;
        f.close();

        conn_string = std::string("dbname=") + dbname + " user=" + user + " password=" + password;
    }
};

crow::response signUp(const crow::json::rvalue& json);
crow::response signIn(const crow::json::rvalue& json);

crow::response filesUpload(const crow::request& req);
crow::response filesList(const crow::request& req);
crow::response filesDelete(int file_id);
void filesDownload(crow::response& res, int file_id);

#endif  // APIREQUESTS_HPP