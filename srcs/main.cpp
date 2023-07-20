#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <mysql_error.h>

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

    // CROW_ROUTE(app, "/test/<int>/<int>")
    //     .methods(crow::HTTPMethod::POST)(
    //         [](int a, int b) { return std::to_string(a + b) + '\n'; });

    CROW_ROUTE(app, "/auth/sign-in")(
        [](const crow::request& req)
        {   
            crow::json::wvalue json = authUser(req);
            return json;
        }
    );
    CROW_ROUTE(app, "/api/v1/files")([] { return "hi\n"; });

    app.port(18080).run();
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
