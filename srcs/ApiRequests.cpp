#include "ApiRequests.hpp"

#include <fstream>
#include <pqxx/pqxx>

#include "HashPasswordToken.hpp"

crow::response signUp(const crow::json::rvalue &json)
{
    try
    {
        std::string user(json["user"].s());
        std::string password(json["password"].s());

        env e;
        pqxx::connection c(e.conn_string);
        pqxx::work w(c);

        c.prepare("find", "SELECT username FROM server.users WHERE username = $1");
        pqxx::result res = w.exec_prepared("find", user);

        if (res.size())
        {
            CROW_LOG_DEBUG << "User already exists!";
            return crow::response(crow::status::BAD_REQUEST);
        }
        else
        {
            c.prepare("insert", "INSERT INTO server.users(username, hash_pass) VALUES ($1, $2)");
            w.exec_prepared0("insert", user, generateHash(password));
            w.commit();

            c.prepare("find_id", "SELECT user_id FROM server.users WHERE username = $1");
            pqxx::row r = w.exec_prepared1("find_id", user);

            int id = r[0].as<int>();

            crow::json::wvalue j;
            j["id"] = id;
            return crow::response(j);
        }
    }
    catch (const pqxx::unexpected_rows &e)
    {
        CROW_LOG_ERROR << e.what();
        return crow::response(crow::status::BAD_REQUEST);
    }
    catch (const std::runtime_error &e)
    {
        CROW_LOG_ERROR << e.what();
        return crow::response(crow::status::BAD_REQUEST);
    }
    catch (...)
    {
        return crow::response(crow::status::INTERNAL_SERVER_ERROR);
    }
}

crow::response signIn(const crow::json::rvalue &json)
{
    try
    {
        std::string user(json["user"].s());
        std::string password(json["password"].s());

        env e;
        pqxx::connection c(e.conn_string);
        pqxx::work w(c);

        c.prepare("find_pass", "SELECT hash_pass FROM server.users WHERE username = $1");

        pqxx::row r = w.exec_prepared1("find_pass", user);
        std::string pass(generateHash(password));

        if (pass != r[0].as<std::string>())
            return crow::response(crow::status::UNAUTHORIZED);

        std::string token(generateToken());
        crow::json::wvalue j;
        j["token"] = token;

        std::string update_query("UPDATE server.users SET hash_token = ? WHERE username = ?;");

        c.prepare("update_token", "UPDATE server.users SET hash_token = $1 WHERE username = $2");
        w.exec_prepared0("update_token", generateHash(token), user);
        w.commit();

        return crow::response(j);
    }
    catch (const pqxx::unexpected_rows &e)
    {
        CROW_LOG_ERROR << "User doesn't exists";
        return crow::response(crow::status::BAD_REQUEST);
    }
    catch (const std::runtime_error &e)
    {
        CROW_LOG_ERROR << e.what();
        return crow::response(crow::status::BAD_REQUEST);
    }
}

crow::response filesUpload(const crow::request &req)
{
    try
    {
        crow::multipart::message file_message(req);
        std::string user_token(req.get_header_value("Authorization").substr(7));

        if (file_message.part_map.size() != 1)
            return crow::response(crow::status::BAD_REQUEST);

        env e;
        pqxx::connection c(e.conn_string);
        pqxx::work w(c);

        c.prepare("find_user_id", "SELECT user_id FROM server.users WHERE hash_token = $1");
        pqxx::row r         = w.exec_prepared1("find_user_id", generateHash(user_token));
        std::string user_id = std::to_string(r[0].as<int>());

        c.prepare("check", "SELECT file_id FROM server.files_info WHERE is_deleted = false AND name = $1 AND user_id = $2");
        c.prepare("find_file_id",
                  "SELECT file_id FROM server.files_info WHERE is_deleted = false AND name = $1 AND user_id = $2");
        c.prepare("insert_files_info", "INSERT INTO server.files_info(name, user_id) VALUES($1, $2)");

        const auto &part       = *file_message.part_map.begin();
        const auto &part_name  = part.first;
        const auto &part_value = part.second;

        if ("InputFile" == part_name)
        {
            auto headers_it = part_value.headers.find("Content-Disposition");
            if (headers_it == part_value.headers.end())
            {
                CROW_LOG_ERROR << "No Content-Disposition found";
                return crow::response(crow::status::BAD_REQUEST);
            }
            auto params_it = headers_it->second.params.find("filename");
            if (params_it == headers_it->second.params.end())
            {
                CROW_LOG_ERROR << "Part with name \"InputFile\" should have a file";
                return crow::response(crow::status::BAD_REQUEST);
            }

            try
            {
                w.exec_prepared0("check", params_it->second, user_id);
                const std::string outfile_name = std::string("./files/") + params_it->second;

                std::ofstream out_file(outfile_name);
                if (!out_file)
                {
                    CROW_LOG_ERROR << " Write to file failed";
                    return crow::response(crow::status::INTERNAL_SERVER_ERROR);
                }
                out_file << part_value.body;
                out_file.close();

                w.exec_prepared0("insert_files_info", params_it->second, user_id);
                w.commit();
                r = w.exec_prepared1("find_file_id", params_it->second, user_id);
            }
            catch (const pqxx::unexpected_rows &e)
            {
                CROW_LOG_ERROR << e.what();
                return crow::response(crow::status::BAD_REQUEST);
            }

            crow::json::wvalue json;
            json["file_id"] = r[0].as<int>();
            return crow::response(json);
        }
        else
            return crow::response(crow::status::BAD_REQUEST);
    }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << e.what();
        return crow::response(crow::status::INTERNAL_SERVER_ERROR);
    }
}

crow::response filesList(const crow::request &req)
{
    std::string user_token(req.get_header_value("Authorization").substr(7));

    try
    {
        env e;
        pqxx::connection c(e.conn_string);
        pqxx::work w(c);

        c.prepare("find_user_id", "SELECT user_id FROM server.users WHERE hash_token = $1");
        pqxx::row r = w.exec_prepared1("find_user_id", generateHash(user_token));

        c.prepare("find_files", "SELECT * FROM server.files_info WHERE is_deleted = false AND user_id = $1");
        pqxx::result res = w.exec_prepared("find_files", r[0].as<int>());

        crow::json::wvalue json;
        for (int i = 0; i < res.size(); ++i)
        {
            json[std::string("InputFile_") + std::to_string(i)] = {
                {"file_id", res[i][0].as<int>()},
                {"name",    res[i][1].c_str()  }
            };
        }
        return crow::response(json);
    }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << e.what();
        return crow::response(crow::status::INTERNAL_SERVER_ERROR);
    }
}

crow::response filesDelete(const crow::request &req)
{
    try
    {
        env e;
        pqxx::connection c(e.conn_string);
        pqxx::work w(c);

        c.prepare("find_file_id", "SELECT is_deleted FROM server.files_info WHERE file_id = $1");

        int file_id = (crow::json::load(req.body))["file_id"].i();
        std::cerr << file_id << std::endl;
        pqxx::row r = w.exec_prepared1("find_file_id", file_id);

        if (!r[0].as<bool>())
        {
            c.prepare("update_file_status", "UPDATE server.files_info SET is_deleted = true WHERE file_id = $1");
            w.exec_prepared0("update_file_status", file_id);
            w.commit();
        }
        else
            return crow::response(crow::status::BAD_REQUEST);
    }
    catch (const pqxx::unexpected_rows &e)
    {
        CROW_LOG_DEBUG << "File doesn't exists";
        return crow::response(crow::status::BAD_REQUEST);
    }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << e.what();
        return crow::response(crow::status::BAD_REQUEST);
    }
    return crow::response(crow::status::OK);
}
