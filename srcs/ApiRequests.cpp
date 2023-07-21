#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <mysql_error.h>
#include <fstream>

#include "ApiRequests.hpp"
#include "HashPasswordToken.hpp"

crow::response signUp(const crow::json::rvalue& json)
{
	try {
		std::string user(json["user"].s());
		std::string password(json["user"].s());

		try
		{
			env e;
			sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
			sql::Connection *conn = driver->connect("tcp://127.0.0.1:3306", e.user, e.pass);

			if (!conn)
				throw "No connection";
			std::string query ("SELECT username FROM server.users WHERE username = ?;");
			sql::PreparedStatement *pstmt = conn->prepareStatement(query);
			pstmt->setString(1, user);
			sql::ResultSet *res = pstmt->executeQuery();
			
			if (res->next())
			{
				std::cerr << "User already exists!" << std::endl;
				conn->close();
				delete conn;
				delete pstmt;
				delete res;
				return crow::response(crow::status::BAD_REQUEST);
			}
			else
			{
				std::string insert_query ("INSERT INTO server.users(username, hash_pass) VALUES (?, ?);");
				pstmt->clearParameters();
				pstmt = conn->prepareStatement(insert_query);
				pstmt->setString(1, user);
				pstmt->setString(2, generateHash(password));
				int code = pstmt->executeUpdate();
				if (code == 0)
				{
					conn->close();
					delete conn;
					delete pstmt;
					delete res;
					throw "Can't insert";
				}
				// pstmt->close();
				query = "SELECT user_id FROM server.users WHERE username = ?;";
				pstmt->clearParameters();
				pstmt = conn->prepareStatement(query);
				pstmt->setString(1, user);

				res = pstmt->executeQuery();
				int id = -1;
				if (res->next())
					id = res->getInt("user_id");
				crow::json::wvalue j;
				j["id"] = id;
				return crow::response(j);
			}
			conn->close();
			delete conn;
			delete pstmt;
			delete res;
		}
		catch (const std::runtime_error &e)
		{
			std::cerr << e.what() << std::endl;
			return crow::response(crow::status::INTERNAL_SERVER_ERROR);
		}
	}
	catch (const std::runtime_error &e)
	{
		std::cerr << "UB " << e.what() << std::endl;
		return crow::response(crow::status::BAD_REQUEST);
	}
	catch (const std::exception &e)
	{
		std::cerr << "ER " << e.what() << std::endl;
		return crow::response(crow::status::INTERNAL_SERVER_ERROR);
	}
}
