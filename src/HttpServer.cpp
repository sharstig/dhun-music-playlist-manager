#include "HttpServer.h"
#include "JsonHelper.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
    #pragma comment(lib, "ws2_32.lib")
#endif

HttpServer::HttpServer(int p, const std::string& staticDirectory)
    : port(p), staticDir(staticDirectory), isRunning(false), serverSocket(INVALID_SOCKET) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

HttpServer::~HttpServer() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

void HttpServer::Get(const std::string& path, HttpHandler handler) {
    getRoutes[path] = handler;
}

void HttpServer::Post(const std::string& path, HttpHandler handler) {
    postRoutes[path] = handler;
}

void HttpServer::Delete(const std::string& path, HttpHandler handler) {
    deleteRoutes[path] = handler;
}

std::string HttpServer::getStatusText(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 409: return "Conflict";
        case 500: return "Internal Server Error";
        default:  return "OK";
    }
}

std::string HttpServer::getMimeType(const std::string& filePath) {
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) return "application/octet-stream";
    std::string ext = filePath.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".html" || ext == ".htm") return "text/html; charset=utf-8";
    if (ext == ".css")  return "text/css; charset=utf-8";
    if (ext == ".js")   return "application/javascript; charset=utf-8";
    if (ext == ".json") return "application/json; charset=utf-8";
    if (ext == ".png")  return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".svg")  return "image/svg+xml";
    if (ext == ".ico")  return "image/x-icon";
    if (ext == ".mp3")  return "audio/mpeg";
    if (ext == ".wav")  return "audio/wav";
    if (ext == ".ogg")  return "audio/ogg";
    return "text/plain; charset=utf-8";
}

bool HttpServer::serveStaticFile(const std::string& reqPath, HttpResponse& res) {
    std::string cleanPath = reqPath;
    if (cleanPath.empty() || cleanPath == "/") {
        cleanPath = "/index.html";
    }

    // Prevent directory traversal attacks
    if (cleanPath.find("..") != std::string::npos) {
        return false;
    }

    std::string fullPath = staticDir + cleanPath;
    std::ifstream file(fullPath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    res.body = ss.str();
    res.status = 200;
    res.contentType = getMimeType(fullPath);
    return true;
}

void HttpServer::handleClient(socket_t clientSocket) {
    std::string rawRequest;
    char buffer[4096];
    int bytesRead = 0;

    // Read headers
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        rawRequest.append(buffer, bytesRead);
        if (rawRequest.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    if (rawRequest.empty()) {
        closesocket(clientSocket);
        return;
    }

    size_t headerEnd = rawRequest.find("\r\n\r\n");
    std::string headerPart = rawRequest.substr(0, headerEnd);
    std::string initialBody = (headerEnd + 4 < rawRequest.length()) ? rawRequest.substr(headerEnd + 4) : "";

    std::istringstream stream(headerPart);
    std::string requestLine;
    std::getline(stream, requestLine);
    if (!requestLine.empty() && requestLine.back() == '\r') {
        requestLine.pop_back();
    }

    std::istringstream reqLineStream(requestLine);
    HttpRequest req;
    std::string rawUrl, httpVersion;
    reqLineStream >> req.method >> rawUrl >> httpVersion;

    // Parse path and query
    size_t qmarkPos = rawUrl.find('?');
    if (qmarkPos != std::string::npos) {
        req.path = rawUrl.substr(0, qmarkPos);
        req.query = rawUrl.substr(qmarkPos + 1);
        req.params = JsonHelper::parseQueryString(req.query);
    } else {
        req.path = rawUrl;
    }

    // Parse headers
    std::string headerLine;
    size_t contentLength = 0;
    while (std::getline(stream, headerLine)) {
        if (!headerLine.empty() && headerLine.back() == '\r') headerLine.pop_back();
        size_t colon = headerLine.find(':');
        if (colon != std::string::npos) {
            std::string key = headerLine.substr(0, colon);
            std::string val = headerLine.substr(colon + 1);
            while (!val.empty() && val.front() == ' ') val.erase(val.begin());
            req.headers[key] = val;

            std::string keyLower = key;
            std::transform(keyLower.begin(), keyLower.end(), keyLower.begin(), ::tolower);
            if (keyLower == "content-length") {
                try { contentLength = std::stoul(val); } catch (...) {}
            }
        }
    }

    // Read remaining body if Content-Length specified
    req.body = initialBody;
    while (req.body.length() < contentLength) {
        int needed = static_cast<int>(contentLength - req.body.length());
        int toRead = std::min(needed, (int)sizeof(buffer));
        bytesRead = recv(clientSocket, buffer, toRead, 0);
        if (bytesRead <= 0) break;
        req.body.append(buffer, bytesRead);
    }

    HttpResponse res;
    // Handle CORS preflight
    if (req.method == "OPTIONS") {
        res.status = 204;
        res.body = "";
    } else if (req.method == "GET" || req.method == "HEAD") {
        auto it = getRoutes.find(req.path);
        if (it != getRoutes.end()) {
            it->second(req, res);
        } else {
            // Check static files
            if (!serveStaticFile(req.path, res)) {
                res.setJson(JsonHelper::errorJson("Route not found: " + req.path, 404), 404);
            }
        }
    } else if (req.method == "POST") {
        auto it = postRoutes.find(req.path);
        if (it != postRoutes.end()) {
            it->second(req, res);
        } else {
            // Check dynamic path like /api/play/:id
            if (req.path.rfind("/api/play/", 0) == 0) {
                std::string idStr = req.path.substr(10);
                req.params["id"] = idStr;
                auto playIt = postRoutes.find("/api/play");
                if (playIt != postRoutes.end()) {
                    playIt->second(req, res);
                } else {
                    res.setJson(JsonHelper::errorJson("Route not found: " + req.path, 404), 404);
                }
            } else {
                res.setJson(JsonHelper::errorJson("Route not found: " + req.path, 404), 404);
            }
        }
    } else if (req.method == "DELETE") {
        auto it = deleteRoutes.find(req.path);
        if (it != deleteRoutes.end()) {
            it->second(req, res);
        } else if (req.path.rfind("/api/songs/", 0) == 0) {
            // Match /api/songs/:id
            std::string idStr = req.path.substr(11);
            req.params["id"] = idStr;
            auto delSongs = deleteRoutes.find("/api/songs");
            if (delSongs != deleteRoutes.end()) {
                delSongs->second(req, res);
            } else {
                res.setJson(JsonHelper::errorJson("Route not found: " + req.path, 404), 404);
            }
        } else {
            res.setJson(JsonHelper::errorJson("Route not found: " + req.path, 404), 404);
        }
    } else {
        res.setJson(JsonHelper::errorJson("Method not supported: " + req.method, 405), 405);
    }

    // Build HTTP response string
    std::ostringstream responseStream;
    responseStream << "HTTP/1.1 " << res.status << " " << getStatusText(res.status) << "\r\n";
    responseStream << "Content-Type: " << res.contentType << "\r\n";
    responseStream << "Content-Length: " << res.body.size() << "\r\n";
    responseStream << "Access-Control-Allow-Origin: *\r\n";
    responseStream << "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n";
    responseStream << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    responseStream << "Connection: close\r\n";

    for (const auto& h : res.headers) {
        responseStream << h.first << ": " << h.second << "\r\n";
    }
    responseStream << "\r\n";
    if (req.method != "HEAD") {
        responseStream << res.body;
    }

    std::string responseStr = responseStream.str();
    send(clientSocket, responseStr.c_str(), static_cast<int>(responseStr.length()), 0);

    closesocket(clientSocket);
}

void HttpServer::start() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "[ERROR] Could not create server socket.\n";
        return;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Bind to 0.0.0.0
    serverAddr.sin_port = htons(static_cast<u_short>(port));

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
#ifdef _WIN32
        std::cerr << "[ERROR] Failed to bind to port " << port << " on 0.0.0.0. WSAError: " << WSAGetLastError() << std::endl;
#else
        std::cerr << "[ERROR] Failed to bind to port " << port << " on 0.0.0.0. Error: " << strerror(errno) << std::endl;
#endif
        closesocket(serverSocket);
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
#ifdef _WIN32
        std::cerr << "[ERROR] Failed to listen on socket. WSAError: " << WSAGetLastError() << std::endl;
#else
        std::cerr << "[ERROR] Failed to listen on socket. Error: " << strerror(errno) << std::endl;
#endif
        closesocket(serverSocket);
        return;
    }

    isRunning = true;
    std::cout << "====================================================\n"
              << " DHUN Music Playlist Server running on 0.0.0.0:" << port << "\n"
              << " Open your browser at: http://localhost:" << port << "\n"
              << "====================================================" << std::endl;

    while (isRunning) {
        sockaddr_in clientAddr;
#ifdef _WIN32
        int clientLen = sizeof(clientAddr);
#else
        socklen_t clientLen = sizeof(clientAddr);
#endif
        socket_t clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket == INVALID_SOCKET) {
            if (!isRunning) break;
            continue;
        }

        // Spawn detached thread to handle request concurrently
        std::thread([this, clientSocket]() {
            this->handleClient(clientSocket);
        }).detach();
    }
}

void HttpServer::stop() {
    isRunning = false;
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
}
