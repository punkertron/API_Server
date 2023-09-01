#ifndef APIREQUESTS_HPP
#define APIREQUESTS_HPP

#include <crow.h>

#include <iostream>
#include <string>

class env
{
   private:
    std::string conn_string;

   public:
    env()
    {
        conn_string = std::string("dbname=") + getenv("POSTGRES_DB") + " user=" + getenv("POSTGRES_USER") +
                      " password=" + getenv("POSTGRES_PASSWORD") + " host=postgres" + " port=5432";
    }

    const std::string& getConnStr() const { return conn_string; }
};

crow::response signUp(const crow::json::rvalue& json);
crow::response signIn(const crow::json::rvalue& json);

crow::response filesUpload(const crow::request& req);
crow::response filesList(const crow::request& req);
crow::response filesDelete(const crow::json::rvalue& json);
crow::response filesDownload(const crow::json::rvalue& json);

#endif  // APIREQUESTS_HPP