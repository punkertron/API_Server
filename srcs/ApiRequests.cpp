#include "ApiRequests.hpp"

#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <mysql_error.h>

#include <fstream>

#include "HashPasswordToken.hpp"

crow::response signUp(const crow::json::rvalue &json) {
    try {
        std::string user(json["user"].s());
        std::string password(json["password"].s());

        try {
            env e;
            sql::mysql::MySQL_Driver *driver =
                sql::mysql::get_mysql_driver_instance();
            sql::Connection *conn =
                driver->connect("tcp://127.0.0.1:3306", e.user, e.pass);

            if (!conn) throw "No connection";
            std::string query(
                "SELECT username FROM server.users WHERE username = ?;");
            sql::PreparedStatement *pstmt = conn->prepareStatement(query);
            pstmt->setString(1, user);
            sql::ResultSet *res = pstmt->executeQuery();

            if (res->next()) {
                std::cerr << "User already exists!" << std::endl;
                conn->close();
                delete conn;
                delete pstmt;
                delete res;
                return crow::response(crow::status::BAD_REQUEST);
            } else {
                std::string insert_query(
                    "INSERT INTO server.users(username, hash_pass) VALUES (?, "
                    "?);");
                delete pstmt;
                pstmt = conn->prepareStatement(insert_query);
                pstmt->setString(1, user);
                pstmt->setString(2, generateHash(password));
                int code = pstmt->executeUpdate();
                if (code == 0) {
                    conn->close();
                    delete conn;
                    delete pstmt;
                    delete res;
                    throw "Can't insert";
                }
                // pstmt->close();
                query = "SELECT user_id FROM server.users WHERE username = ?;";
                delete pstmt;
                pstmt = conn->prepareStatement(query);
                pstmt->setString(1, user);

                res = pstmt->executeQuery();
                int id = -1;
                if (res->next()) id = res->getInt("user_id");
                crow::json::wvalue j;
                j["id"] = id;
                return crow::response(j);
            }
            conn->close();
            delete conn;
            delete pstmt;
            delete res;
        } catch (const std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
            return crow::response(crow::status::INTERNAL_SERVER_ERROR);
        }
    } catch (const std::runtime_error &e) {
        std::cerr << "UB " << e.what() << std::endl;
        return crow::response(crow::status::BAD_REQUEST);
    } catch (const std::exception &e) {
        std::cerr << "ER " << e.what() << std::endl;
        return crow::response(crow::status::INTERNAL_SERVER_ERROR);
    }
}

// without errors
crow::response signIn(const crow::json::rvalue &json) {
    try {
        std::string user(json["user"].s());
        std::string password(json["password"].s());

        env e;
        sql::mysql::MySQL_Driver *driver =
            sql::mysql::get_mysql_driver_instance();
        sql::Connection *conn =
            driver->connect("tcp://127.0.0.1:3306", e.user, e.pass);

        std::string query(
            "SELECT hash_pass FROM server.users WHERE username = ?;");
        sql::PreparedStatement *pstmt = conn->prepareStatement(query);
        pstmt->setString(1, user);
        sql::ResultSet *res = pstmt->executeQuery();

        if (res->next()) {
            std::string pass(generateHash(password));

            if (pass != res->getString("hash_pass")) {
                conn->close();
                delete conn;
                delete pstmt;
                delete res;
                return crow::response(crow::status::UNAUTHORIZED);
            }

            std::string token(generateToken());
            crow::json::wvalue j;
            j["token"] = token;

            std::string update_query(
                "UPDATE server.users SET hash_token = ? WHERE username = ?;");

            delete pstmt;
            pstmt = conn->prepareStatement(update_query);
            pstmt->setString(1, generateHash(token));
            pstmt->setString(2, user);
            pstmt->executeUpdate();

            delete pstmt;
            conn->close();
            delete conn;
            return crow::response(j);
        } else {
            std::cerr << "User doesn't exists" << std::endl;
            conn->close();
            delete conn;
            delete pstmt;
            delete res;
            return crow::response(crow::status::BAD_REQUEST);
        }

    } catch (const std::runtime_error &e) {
        std::cerr << "UB " << e.what() << std::endl;
        return crow::response(crow::status::BAD_REQUEST);
    }
}

crow::response uploadFiles(const crow::request &req) {
    crow::multipart::message file_message(req);
    for (const auto &part : file_message.part_map) {
        const auto &part_name = part.first;
        const auto &part_value = part.second;

        if ("InputFile" == part_name) {
            auto headers_it = part_value.headers.find("Content-Disposition");
            if (headers_it == part_value.headers.end()) {
                CROW_LOG_ERROR << "No Content-Disposition found";
                return crow::response(400);
            }
            auto params_it = headers_it->second.params.find("filename");
            if (params_it == headers_it->second.params.end()) {
                CROW_LOG_ERROR
                    << "Part with name \"InputFile\" should have a file";
                return crow::response(400);
            }
            const std::string outfile_name =
                std::string("./files/") + params_it->second;

            for (const auto &part_header : part_value.headers) {
                const auto &part_header_name = part_header.first;
                const auto &part_header_val = part_header.second;
                CROW_LOG_DEBUG << "Header: " << part_header_name << '='
                               << part_header_val.value;
                for (const auto &param : part_header_val.params) {
                    const auto &param_key = param.first;
                    const auto &param_val = param.second;
                    CROW_LOG_DEBUG << " Param: " << param_key << ','
                                   << param_val;
                }
            }

            // Create a new file with the extracted file name and write file
            // contents to it
            std::ofstream out_file(outfile_name);
            if (!out_file) {
                CROW_LOG_ERROR << " Write to file failed";
                continue;
            }
            out_file << part_value.body;
            out_file.close();
            CROW_LOG_INFO << " Contents written to " << outfile_name;
        }
    }
    return crow::response(crow::status::OK);
}
