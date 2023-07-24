#include <fstream>
#include <iostream>
#include <string>

#include "ApiRequests.hpp"
#include "Authorization.hpp"
#include "ContentType.hpp"
#include "crow_all.h"

int main()
{
    crow::App<ContentTypeJson, ContentTypeMultipart, Authorization> app;
    app.loglevel(crow::LogLevel::INFO);  // default

    CROW_ROUTE(app, "/").CROW_MIDDLEWARES(app, ContentTypeJson, Authorization)(
        []()
        {
            return "Hello world\n";
        });

    CROW_ROUTE(app, "/auth/sign-up")
        .CROW_MIDDLEWARES(app, ContentTypeJson)(
            [](const crow::request& req)
            {
                crow::json::rvalue json = crow::json::load(req.body);
                if (!json)
                    return crow::response(crow::status::BAD_REQUEST);
                return signUp(json);
            });

    CROW_ROUTE(app, "/auth/sign-in")
        .CROW_MIDDLEWARES(app, ContentTypeJson)(
            [](const crow::request& req)
            {
                crow::json::rvalue json = crow::json::load(req.body);
                if (!json)
                    return crow::response(crow::status::BAD_REQUEST);
                return signIn(json);
            });

    CROW_ROUTE(app, "/api/v1/files/upload")
        .methods(crow::HTTPMethod::POST)
        .CROW_MIDDLEWARES(app, /*ContentTypeMultipart,*/ Authorization)(
            [](const crow::request& req)
            {
                return filesUpload(req);
            });

    CROW_ROUTE(app, "/api/v1/files/list")
        .CROW_MIDDLEWARES(app, Authorization)(
            [](const crow::request& req)
            {
                return filesList(req);
            });

    CROW_ROUTE(app, "/api/v1/files/delete")
        .methods(crow::HTTPMethod::DELETE)
        .CROW_MIDDLEWARES(app, ContentTypeJson, Authorization)(
            [](const crow::request& req)
            {
                return filesDelete(req);
            });

    app.port(18080).multithreaded().run();
}

// CROW_ROUTE(app, "/test/<int>/<int>")
//     .methods(crow::HTTPMethod::POST)(
//         [](int a, int b) { return std::to_string(a + b) + '\n'; });