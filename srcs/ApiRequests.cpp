#include "ApiRequests.hpp"

#include <fstream>
#include <pqxx/pqxx>

#include "HashPasswordToken.hpp"

crow::response signUp(const crow::json::rvalue &json) {
    try {
        std::string user(json["user"].s());
        std::string password(json["password"].s());

        try {
            env e;

            pqxx::connection c(e.conn_string);
            pqxx::work w(c);

            c.prepare("find",
                      "SELECT username FROM server.users WHERE username = $1");
            pqxx::result res = w.exec_prepared("find", user);

            if (res.size()) {
                std::cerr << "User already exists!" << std::endl;
                c.close();
                return crow::response(crow::status::BAD_REQUEST);
            } else {
                c.prepare("insert",
                          "INSERT INTO server.users(username, hash_pass) "
                          "VALUES ($1, $2)");
                try {
                    w.exec_prepared0("insert", user, generateHash(password));
                    w.commit();

                    c.prepare(
                        "find_id",
                        "SELECT user_id FROM server.users WHERE username = $1");
                    pqxx::row r = w.exec_prepared1("find_id", user);

                    int id = r[0].as<int>();

                    crow::json::wvalue j;
                    j["id"] = id;
                    return crow::response(j);
                } catch (const pqxx::unexpected_rows &e) {
                    c.close();
                    std::cerr << e.what() << std::endl;
                    return crow::response(crow::status::BAD_REQUEST);
                }
            }
        } catch (const std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
            return crow::response(crow::status::INTERNAL_SERVER_ERROR);
        }
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return crow::response(crow::status::BAD_REQUEST);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return crow::response(crow::status::INTERNAL_SERVER_ERROR);
    }
}

// without errors
crow::response signIn(const crow::json::rvalue &json) {
    try {
        std::string user(json["user"].s());
        std::string password(json["password"].s());

        env e;
        pqxx::connection c(e.conn_string);
        pqxx::work w(c);

        c.prepare("find_pass",
                  "SELECT hash_pass FROM server.users WHERE username = $1");

        try {
            pqxx::row r = w.exec_prepared1("find_pass", user);
            std::string pass(generateHash(password));

            if (pass != r[0].as<std::string>()) {
                c.close();
                return crow::response(crow::status::UNAUTHORIZED);
            }

            std::string token(generateToken());
            crow::json::wvalue j;
            j["token"] = token;

            std::string update_query(
                "UPDATE server.users SET hash_token = ? WHERE username = ?;");

            c.prepare(
                "update_token",
                "UPDATE server.users SET hash_token = $1 WHERE username = $2");
            w.exec_prepared0("update_token", generateHash(token), user);
            w.commit();

            c.close();
            return crow::response(j);
        } catch (const pqxx::unexpected_rows &e) {
            std::cerr << "User doesn't exists" << std::endl;
            c.close();
            return crow::response(crow::status::BAD_REQUEST);
        }

    } catch (const std::runtime_error &e) {
        std::cerr << "UB " << e.what() << std::endl;
        return crow::response(crow::status::BAD_REQUEST);
    }
}

crow::response filesUpload(const crow::request &req) {
    crow::multipart::message file_message(req);
    std::string user_token(req.get_header_value("Authorization").substr(7));
    crow::json::wvalue return_json;

    try {
        env e;

        pqxx::connection c(e.conn_string);
        pqxx::work w(c);

        c.prepare("find_user_id",
                  "SELECT user_id FROM server.users WHERE hash_token = $1");
        pqxx::row r =
            w.exec_prepared1("find_user_id", generateHash(user_token));
        std::string user_id = std::to_string(r[0].as<int>());

        c.prepare("check",
                              "SELECT file_id FROM server.files_info WHERE "
                              "name = $1 AND user_id = $2");
        
        c.prepare("find_file_id",
                              "SELECT file_id FROM server.files_info WHERE "
                              "name = $1 AND user_id = $2");

        c.prepare("insert_files_info",
                              "INSERT INTO server.files_info(name, user_id) "
                              "VALUES($1, $2)");

        int file_number = 0;
        for (const auto &part : file_message.part_map) {
            const auto &part_name = part.first;
            const auto &part_value = part.second;

            if ("InputFile" == part_name) {
                auto headers_it =
                    part_value.headers.find("Content-Disposition");
                if (headers_it == part_value.headers.end()) {
                    CROW_LOG_ERROR << "No Content-Disposition found";
                    return crow::response(crow::status::BAD_REQUEST);
                }
                auto params_it = headers_it->second.params.find("filename");
                if (params_it == headers_it->second.params.end()) {
                    CROW_LOG_ERROR
                        << "Part with name \"InputFile\" should have a file";
                    return crow::response(crow::status::BAD_REQUEST);
                }

                // TODO: if user try to upload several files, but one or more of them exist what should I do?
                try {
                    w.exec_prepared0("check", params_it->second, user_id);

                    w.exec_prepared0("insert_files_info", params_it->second,
                                     user_id);
                    w.commit();

                    r = w.exec_prepared1("find_file_id", params_it->second,
                                         user_id);
                } catch (const pqxx::unexpected_rows &e) {
                    std::cerr << e.what() << std::endl;
                    c.close();
                    return crow::response(crow::status::BAD_REQUEST);
                }
                return_json[std::string("InputFile_") + std::to_string(file_number)] = {
                    {"file_id", r[0].as<int>()},
                    {"file_name", params_it->second}
                };

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

                ++file_number;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return crow::response(crow::status::INTERNAL_SERVER_ERROR);
    }
    return crow::response(return_json);
}


crow::response filesList(const crow::request &req)
{
    std::string user_token(req.get_header_value("Authorization").substr(7));

    try {
        env e;

        pqxx::connection c(e.conn_string);
        pqxx::work w(c);

        c.prepare("find_user_id", "SELECT user_id FROM server.users WHERE hash_token = $1");
        pqxx::row r = w.exec_prepared1("find_user_id", generateHash(user_token));

        c.prepare("find_files", "SELECT * FROM server.files_info WHERE user_id = $1");
        pqxx::result res = w.exec_prepared("find_files", r[0].as<int>());

        crow::json::wvalue json;
       // int i = 0;

        for (int i = 0; i < res.size(); ++i)
        {
            json[std::string("InputFile_") + std::to_string(i)] = {
                {"file_id", res[i][0].as<int>()},
                {"name", res[i][1].c_str()},
                {"is_deleted", res[i][2].as<bool>()}
            };
        }
        return crow::response(json);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        // c.close();
        return crow::response(crow::status::INTERNAL_SERVER_ERROR);
    }
}