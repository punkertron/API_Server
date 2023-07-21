#include <fstream>
#include <iostream>
#include <string>

#include "crow.h"
#include "Authorization.hpp"
#include "ApiRequests.hpp"

int main() {

    crow::App<Authorization> app;
    app.loglevel(crow::LogLevel::INFO);  // default

    CROW_ROUTE(app, "/")([]() { return "Hello world\n"; });

    CROW_ROUTE(app, "/auth/sign-up")(
        [](const crow::request& req)
        {   
            crow::json::rvalue json = crow::json::load(req.body);
            if (!json)
                return crow::response(crow::status::BAD_REQUEST);
            return signUp(json);
        }
    );

    app.port(18080).multithreaded().run();
}

// sql::mysql::MySQL_Driver *driver;
// sql::Connection *conn;

// std::ifstream f;  // TODO : replace with getenv()
// f.open(".env");
// std::string user, pass;
// f >> user >> pass;
// f.close();

// try {
//     driver = sql::mysql::get_mysql_driver_instance();
//     conn = driver->connect("tcp://127.0.0.1:3306", user, pass);

//     sql::Statement *stmt = conn->createStatement();
//     std::string query = R"(
//         CREATE TABLE server.tab (
//             ID int NOT NULL AUTO_INCREMENT,
//             test VARCHAR(100),
//             PRIMARY KEY (ID)
//         );
//     )";
//     sql::ResultSet *res = stmt->executeQuery(std::move(query));
//     while (res->next()) {
//         std::string name = res->getString("_message");
//         std::cout << name << std::endl;
//     }

// } catch (const std::exception &e) {
//     std::cerr << e.what() << std::endl;
// }

// CROW_ROUTE(app, "/test/<int>/<int>")
    //     .methods(crow::HTTPMethod::POST)(
    //         [](int a, int b) { return std::to_string(a + b) + '\n'; });