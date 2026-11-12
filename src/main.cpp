#include "DoublyCircularList.h"
#include "HttpServer.h"
#include "JsonHelper.h"
#include <iostream>
#include <sstream>
#include <mutex>
#include <cstdlib>
#include <string>

int main(int argc, char* argv[]) {
    // Determine port from PORT environment variable (for Render / cloud hosts) or default 8080
    int port = 8080;
    const char* envPort = std::getenv("PORT");
    if (envPort != nullptr && *envPort != '\0') {
        try {
            port = std::stoi(envPort);
        } catch (...) {
            port = 8080;
        }
    } else if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            port = 8080;
        }
    }

    // Core Doubly Circular Linked List instance
    DoublyCircularLinkedList playlist;
    playlist.seedDefaultSongs();

    // Mutex to serialize access across concurrent HTTP worker threads
    std::mutex playlistMutex;

    // Initialize HTTP Server serving static files from ./public
    HttpServer server(port, "./public");

    // 1. Health Check Endpoint (Required for Render & monitoring)
    server.Get("/health", [&playlist, &playlistMutex](const HttpRequest&, HttpResponse& res) {
        std::lock_guard<std::mutex> lock(playlistMutex);
        int currentId = playlist.getCurrent() ? playlist.getCurrent()->id : -1;
        std::ostringstream ss;
        ss << "{"
           << "\"status\":\"healthy\","
           << "\"service\":\"dhun-backend\","
           << "\"songCount\":" << playlist.getSongCount() << ","
           << "\"currentSongId\":" << currentId << ","
           << "\"isCircular\":" << (playlist.verifyInvariants() ? "true" : "false")
           << "}";
        res.setJson(ss.str(), 200);
    });

    // 2. GET /api/songs - Retrieve all songs and circular playlist metadata
    server.Get("/api/songs", [&playlist, &playlistMutex](const HttpRequest&, HttpResponse& res) {
        std::lock_guard<std::mutex> lock(playlistMutex);
        auto songs = playlist.getAllSongs();
        int currentId = playlist.getCurrent() ? playlist.getCurrent()->id : -1;
        int headId = playlist.getHead() ? playlist.getHead()->id : -1;
        int tailId = playlist.getTail() ? playlist.getTail()->id : -1;
        res.setJson(JsonHelper::songsToJson(songs, currentId, headId, tailId, playlist.getSongCount()), 200);
    });

    // 3. GET /api/current - Retrieve current playing song
    server.Get("/api/current", [&playlist, &playlistMutex](const HttpRequest&, HttpResponse& res) {
        std::lock_guard<std::mutex> lock(playlistMutex);
        SongDTO currentDTO;
        if (playlist.getCurrentSongDTO(currentDTO)) {
            res.setJson(JsonHelper::songToJson(currentDTO), 200);
        } else {
            res.setJson("{\"success\":false,\"message\":\"Playlist is empty.\"}", 200);
        }
    });

    // 4. POST /api/songs - Add a new song to the playlist
    server.Post("/api/songs", [&playlist, &playlistMutex](const HttpRequest& req, HttpResponse& res) {
        std::string title, artist, coverUrl, audioUrl;
        int duration = 0;
        int customId = 0;

        // Parse fields from request JSON body
        JsonHelper::getString(req.body, "title", title);
        JsonHelper::getString(req.body, "artist", artist);
        JsonHelper::getInt(req.body, "duration", duration);
        JsonHelper::getInt(req.body, "id", customId);
        JsonHelper::getString(req.body, "coverUrl", coverUrl);
        JsonHelper::getString(req.body, "audioUrl", audioUrl);

        // Validation
        if (title.empty()) {
            res.setJson(JsonHelper::errorJson("Title cannot be empty.", 400), 400);
            return;
        }
        if (artist.empty()) {
            res.setJson(JsonHelper::errorJson("Artist cannot be empty.", 400), 400);
            return;
        }
        if (duration <= 0) {
            res.setJson(JsonHelper::errorJson("Duration must be a positive integer in seconds.", 400), 400);
            return;
        }

        std::lock_guard<std::mutex> lock(playlistMutex);

        if (customId > 0 && playlist.searchById(customId) != nullptr) {
            res.setJson(JsonHelper::errorJson("Song with ID " + std::to_string(customId) + " already exists.", 409), 409);
            return;
        }

        SongNode* added = playlist.addSong(title, artist, duration, customId, coverUrl, audioUrl);
        if (added == nullptr) {
            res.setJson(JsonHelper::errorJson("Failed to add song due to invalid parameters.", 400), 400);
            return;
        }

        SongDTO dto;
        dto.id = added->id;
        dto.title = added->title;
        dto.artist = added->artist;
        dto.duration = added->duration;
        dto.coverUrl = added->coverUrl;
        dto.audioUrl = added->audioUrl;
        dto.prevId = added->prev ? added->prev->id : -1;
        dto.nextId = added->next ? added->next->id : -1;
        dto.isCurrent = (added == playlist.getCurrent());
        dto.isHead = (added == playlist.getHead());
        dto.isTail = (added == playlist.getTail());

        res.setJson(JsonHelper::songToJson(dto), 201);
    });

    // 5. DELETE /api/songs - Delete song by ID or Title
    server.Delete("/api/songs", [&playlist, &playlistMutex](const HttpRequest& req, HttpResponse& res) {
        std::string idStr = req.getParam("id");
        std::string titleStr = req.getParam("title");

        // Also fallback to parsing body if query params are not used
        if (idStr.empty() && !req.body.empty()) {
            int bodyId = 0;
            if (JsonHelper::getInt(req.body, "id", bodyId)) {
                idStr = std::to_string(bodyId);
            }
        }
        if (titleStr.empty() && !req.body.empty()) {
            JsonHelper::getString(req.body, "title", titleStr);
        }

        if (idStr.empty() && titleStr.empty()) {
            res.setJson(JsonHelper::errorJson("Missing 'id' or 'title' parameter for deletion.", 400), 400);
            return;
        }

        std::lock_guard<std::mutex> lock(playlistMutex);

        if (!idStr.empty()) {
            int id = 0;
            try { id = std::stoi(idStr); } catch (...) {
                res.setJson(JsonHelper::errorJson("Invalid ID format.", 400), 400);
                return;
            }

            bool deleted = playlist.deleteSongById(id);
            if (deleted) {
                res.setJson(JsonHelper::successJson("Song ID " + idStr + " deleted successfully."), 200);
            } else {
                res.setJson(JsonHelper::errorJson("Song with ID " + idStr + " not found.", 404), 404);
            }
        } else {
            bool deleted = playlist.deleteSongByTitle(titleStr);
            if (deleted) {
                res.setJson(JsonHelper::successJson("Song '" + titleStr + "' deleted successfully."), 200);
            } else {
                res.setJson(JsonHelper::errorJson("Song with title '" + titleStr + "' not found.", 404), 404);
            }
        }
    });

    // 6. GET /api/songs/search - Search songs by title, artist or ID
    server.Get("/api/songs/search", [&playlist, &playlistMutex](const HttpRequest& req, HttpResponse& res) {
        std::string query = req.getParam("q", req.getParam("query"));
        if (query.empty()) {
            res.setJson(JsonHelper::errorJson("Query parameter 'q' or 'query' is required.", 400), 400);
            return;
        }

        std::lock_guard<std::mutex> lock(playlistMutex);
        auto results = playlist.searchAll(query);
        int currentId = playlist.getCurrent() ? playlist.getCurrent()->id : -1;
        int headId = playlist.getHead() ? playlist.getHead()->id : -1;
        int tailId = playlist.getTail() ? playlist.getTail()->id : -1;
        res.setJson(JsonHelper::songsToJson(results, currentId, headId, tailId, static_cast<int>(results.size())), 200);
    });

    // 7. POST /api/next - Forward circular navigation
    server.Post("/api/next", [&playlist, &playlistMutex](const HttpRequest&, HttpResponse& res) {
        std::lock_guard<std::mutex> lock(playlistMutex);
        SongNode* nextNode = playlist.nextSong();
        if (nextNode != nullptr) {
            SongDTO dto;
            playlist.getCurrentSongDTO(dto);
            res.setJson(JsonHelper::songToJson(dto), 200);
        } else {
            res.setJson(JsonHelper::errorJson("Cannot navigate next: Playlist is empty.", 400), 400);
        }
    });

    // 8. POST /api/prev - Backward circular navigation
    server.Post("/api/prev", [&playlist, &playlistMutex](const HttpRequest&, HttpResponse& res) {
        std::lock_guard<std::mutex> lock(playlistMutex);
        SongNode* prevNode = playlist.prevSong();
        if (prevNode != nullptr) {
            SongDTO dto;
            playlist.getCurrentSongDTO(dto);
            res.setJson(JsonHelper::songToJson(dto), 200);
        } else {
            res.setJson(JsonHelper::errorJson("Cannot navigate prev: Playlist is empty.", 400), 400);
        }
    });

    // 9. POST /api/play - Select song by ID to set as current
    server.Post("/api/play", [&playlist, &playlistMutex](const HttpRequest& req, HttpResponse& res) {
        std::string idStr = req.getParam("id");
        if (idStr.empty() && !req.body.empty()) {
            int bodyId = 0;
            if (JsonHelper::getInt(req.body, "id", bodyId)) {
                idStr = std::to_string(bodyId);
            }
        }

        if (idStr.empty()) {
            res.setJson(JsonHelper::errorJson("Missing 'id' parameter.", 400), 400);
            return;
        }

        int id = 0;
        try { id = std::stoi(idStr); } catch (...) {
            res.setJson(JsonHelper::errorJson("Invalid song ID.", 400), 400);
            return;
        }

        std::lock_guard<std::mutex> lock(playlistMutex);
        SongNode* current = playlist.setCurrentSong(id);
        if (current != nullptr) {
            SongDTO dto;
            playlist.getCurrentSongDTO(dto);
            res.setJson(JsonHelper::songToJson(dto), 200);
        } else {
            res.setJson(JsonHelper::errorJson("Song ID " + idStr + " not found in playlist.", 404), 404);
        }
    });

    // 10. POST /api/reset - Re-seed default demo playlist
    server.Post("/api/reset", [&playlist, &playlistMutex](const HttpRequest&, HttpResponse& res) {
        std::lock_guard<std::mutex> lock(playlistMutex);
        playlist.seedDefaultSongs();
        res.setJson(JsonHelper::successJson("Playlist reset to default demo tracks successfully."), 200);
    });

    // 11. GET /api/dsa-state - Real-time Doubly Circular Linked List pointer graph
    server.Get("/api/dsa-state", [&playlist, &playlistMutex](const HttpRequest&, HttpResponse& res) {
        std::lock_guard<std::mutex> lock(playlistMutex);
        res.setJson(JsonHelper::dsaStateJson(playlist), 200);
    });

    // Start server loop
    server.start();

    return 0;
}
