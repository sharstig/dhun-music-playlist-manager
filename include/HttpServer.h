#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include <map>
#include <functional>
#include <vector>
#include <atomic>
#include <memory>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

struct HttpRequest {
    std::string method;
    std::string path;
    std::string query;
    std::string body;
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> headers;

    std::string getParam(const std::string& key, const std::string& fallback = "") const {
        auto it = params.find(key);
        return it != params.end() ? it->second : fallback;
    }
};

struct HttpResponse {
    int status = 200;
    std::string contentType = "application/json; charset=utf-8";
    std::string body;
    std::map<std::string, std::string> headers;

    void setJson(const std::string& jsonBody, int statusCode = 200) {
        status = statusCode;
        contentType = "application/json; charset=utf-8";
        body = jsonBody;
    }

    void setText(const std::string& text, int statusCode = 200) {
        status = statusCode;
        contentType = "text/plain; charset=utf-8";
        body = text;
    }

    void setHtml(const std::string& html, int statusCode = 200) {
        status = statusCode;
        contentType = "text/html; charset=utf-8";
        body = html;
    }
};

using HttpHandler = std::function<void(const HttpRequest&, HttpResponse&)>;

class HttpServer {
public:
    HttpServer(int port = 8080, const std::string& staticDirectory = "./public");
    ~HttpServer();

    // Route registration
    void Get(const std::string& path, HttpHandler handler);
    void Post(const std::string& path, HttpHandler handler);
    void Delete(const std::string& path, HttpHandler handler);

    // Set static directory
    void setStaticDir(const std::string& dir) { staticDir = dir; }

    // Start server (blocking loop)
    void start();

    // Stop server
    void stop();

private:
    int port;
    std::string staticDir;
    std::atomic<bool> isRunning;
    socket_t serverSocket;

    std::map<std::string, HttpHandler> getRoutes;
    std::map<std::string, HttpHandler> postRoutes;
    std::map<std::string, HttpHandler> deleteRoutes;

    void handleClient(socket_t clientSocket);
    bool serveStaticFile(const std::string& reqPath, HttpResponse& res);
    static std::string getMimeType(const std::string& filePath);
    static std::string getStatusText(int code);
};

#endif // HTTP_SERVER_H
