#ifndef APIREQUESTS_HPP
#define APIREQUESTS_HPP

#include "crow.h"

crow::json::wvalue authUser(const crow::request& req);

#endif // APIREQUESTS_HPP