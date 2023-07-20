#include "ApiRequests.hpp"

crow::json::wvalue authUser(const crow::request& req)
{
	crow::json::wvalue json(req.body);

	std::cout << json["user"] << std::endl;
	return json;
}