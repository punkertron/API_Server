#include <crow.h>

#include <string>

#include "ApiRequests.hpp"
#include "Authorization.hpp"
#include "ContentType.hpp"

int main()
{
    crow::App<ContentTypeJson, ContentTypeMultipart, Authorization> app;
    app.loglevel(crow::LogLevel::INFO);  // default

    CROW_ROUTE(app, "/auth/sign-up")
        .methods(crow::HTTPMethod::POST)
        .CROW_MIDDLEWARES(app, ContentTypeJson)(
            [](const crow::request& req)
            {
                crow::json::rvalue json = crow::json::load(req.body);
                if (!json)
                    return crow::response(crow::status::BAD_REQUEST);
                return signUp(json);
            });

    CROW_ROUTE(app, "/auth/sign-in")
        .methods(crow::HTTPMethod::POST)
        .CROW_MIDDLEWARES(app, ContentTypeJson)(
            [](const crow::request& req)
            {
                crow::json::rvalue json = crow::json::load(req.body);
                if (!json)
                    return crow::response(crow::status::BAD_REQUEST);
                return signIn(json);
            });

    CROW_ROUTE(app, "/api/v1/files")
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

    CROW_ROUTE(app, "/api/v1/files")
        .methods(crow::HTTPMethod::DELETE)
        .CROW_MIDDLEWARES(app, Authorization, ContentTypeJson)(
            [](const crow::request& req)
            {
                crow::json::rvalue json = crow::json::load(req.body);
                if (!json)
                    return crow::response(crow::status::BAD_REQUEST);
                return filesDelete(json);
            });

    CROW_ROUTE(app, "/api/v1/files")
        .methods(crow::HTTPMethod::GET)
        .CROW_MIDDLEWARES(app, Authorization)(
            [](const crow::request& req)
            {
                crow::json::rvalue json = crow::json::load(req.body);
                if (!json)
                    return crow::response(crow::status::BAD_REQUEST);
                return filesDownload(json);
            });

    app.port(std::stoi(getenv("SERVER_PORT"))).multithreaded().run();
}
