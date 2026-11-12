#include "JsonHelper.h"
#include <sstream>
#include <iomanip>
#include <cctype>

namespace JsonHelper {

std::string escape(const std::string& input) {
    std::ostringstream ss;
    for (char c : input) {
        switch (c) {
            case '"':  ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\b': ss << "\\b";  break;
            case '\f': ss << "\\f";  break;
            case '\n': ss << "\\n";  break;
            case '\r': ss << "\\r";  break;
            case '\t': ss << "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                } else {
                    ss << c;
                }
        }
    }
    return ss.str();
}

std::string songToJson(const SongDTO& song) {
    std::ostringstream ss;
    ss << "{"
       << "\"id\":" << song.id << ","
       << "\"title\":\"" << escape(song.title) << "\","
       << "\"artist\":\"" << escape(song.artist) << "\","
       << "\"duration\":" << song.duration << ","
       << "\"coverUrl\":\"" << escape(song.coverUrl) << "\","
       << "\"audioUrl\":\"" << escape(song.audioUrl) << "\","
       << "\"prevId\":" << song.prevId << ","
       << "\"nextId\":" << song.nextId << ","
       << "\"isCurrent\":" << (song.isCurrent ? "true" : "false") << ","
       << "\"isHead\":" << (song.isHead ? "true" : "false") << ","
       << "\"isTail\":" << (song.isTail ? "true" : "false")
       << "}";
    return ss.str();
}

std::string songsToJson(const std::vector<SongDTO>& songs, int currentId, int headId, int tailId, int count) {
    std::ostringstream ss;
    ss << "{"
       << "\"success\":true,"
       << "\"count\":" << count << ","
       << "\"headId\":" << headId << ","
       << "\"tailId\":" << tailId << ","
       << "\"currentId\":" << currentId << ","
       << "\"songs\":[";
    for (size_t i = 0; i < songs.size(); ++i) {
        if (i > 0) ss << ",";
        ss << songToJson(songs[i]);
    }
    ss << "]}";
    return ss.str();
}

std::string errorJson(const std::string& message, int statusCode) {
    std::ostringstream ss;
    ss << "{"
       << "\"success\":false,"
       << "\"statusCode\":" << statusCode << ","
       << "\"error\":\"" << escape(message) << "\""
       << "}";
    return ss.str();
}

std::string successJson(const std::string& message) {
    std::ostringstream ss;
    ss << "{"
       << "\"success\":true,"
       << "\"message\":\"" << escape(message) << "\""
       << "}";
    return ss.str();
}

std::string dsaStateJson(const DoublyCircularLinkedList& list) {
    std::ostringstream ss;
    int headId = list.getHead() ? list.getHead()->id : -1;
    int tailId = list.getTail() ? list.getTail()->id : -1;
    int currentId = list.getCurrent() ? list.getCurrent()->id : -1;
    int count = list.getSongCount();
    bool isCircular = list.verifyInvariants();

    ss << "{"
       << "\"success\":true,"
       << "\"count\":" << count << ","
       << "\"headId\":" << headId << ","
       << "\"tailId\":" << tailId << ","
       << "\"currentId\":" << currentId << ","
       << "\"isCircular\":" << (isCircular ? "true" : "false") << ","
       << "\"nodes\":[";

    std::vector<SongDTO> songs = list.getAllSongs();
    for (size_t i = 0; i < songs.size(); ++i) {
        if (i > 0) ss << ",";
        ss << songToJson(songs[i]);
    }
    ss << "]}";
    return ss.str();
}

bool getString(const std::string& json, const std::string& key, std::string& out) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) {
        pos = json.find(key);
        if (pos == std::string::npos) return false;
        pos += key.length();
    } else {
        pos += key.length() + 2;
    }

    // skip whitespace and colon
    while (pos < json.length() && (std::isspace(json[pos]) || json[pos] == ':')) {
        pos++;
    }
    if (pos >= json.length()) return false;

    // Check if value is enclosed in quotes
    if (json[pos] == '"' || json[pos] == '\'') {
        char quote = json[pos++];
        std::string val;
        while (pos < json.length()) {
            if (json[pos] == '\\' && pos + 1 < json.length()) {
                val += json[pos + 1];
                pos += 2;
            } else if (json[pos] == quote) {
                out = val;
                return true;
            } else {
                val += json[pos++];
            }
        }
        return false;
    } else {
        std::string val;
        while (pos < json.length() && json[pos] != ',' && json[pos] != '}' && json[pos] != '\r' && json[pos] != '\n') {
            val += json[pos++];
        }
        while (!val.empty() && std::isspace(val.back())) val.pop_back();
        out = val;
        return !out.empty();
    }
}

bool getInt(const std::string& json, const std::string& key, int& out) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) {
        pos = json.find(key);
        if (pos == std::string::npos) return false;
        pos += key.length();
    } else {
        pos += key.length() + 2;
    }

    while (pos < json.length() && (std::isspace(json[pos]) || json[pos] == ':')) {
        pos++;
    }
    if (pos >= json.length()) return false;

    if (json[pos] == '"' || json[pos] == '\'') {
        pos++;
    }

    std::string digits;
    while (pos < json.length() && (std::isdigit(json[pos]) || json[pos] == '-')) {
        digits += json[pos++];
    }
    if (!digits.empty()) {
        try {
            out = std::stoi(digits);
            return true;
        } catch (...) { return false; }
    }
    return false;
}

std::string urlDecode(const std::string& src) {
    std::string res;
    res.reserve(src.length());
    for (size_t i = 0; i < src.length(); ++i) {
        if (src[i] == '%' && i + 2 < src.length() && std::isxdigit(src[i + 1]) && std::isxdigit(src[i + 2])) {
            int ch;
            std::istringstream iss(src.substr(i + 1, 2));
            if (iss >> std::hex >> ch) {
                res += static_cast<char>(ch);
                i += 2;
            } else {
                res += src[i];
            }
        } else if (src[i] == '+') {
            res += ' ';
        } else {
            res += src[i];
        }
    }
    return res;
}

std::map<std::string, std::string> parseQueryString(const std::string& query) {
    std::map<std::string, std::string> params;
    std::string q = query;
    if (!q.empty() && q[0] == '?') {
        q = q.substr(1);
    }
    std::istringstream ss(q);
    std::string pair;
    while (std::getline(ss, pair, '&')) {
        size_t eqPos = pair.find('=');
        if (eqPos != std::string::npos) {
            std::string k = urlDecode(pair.substr(0, eqPos));
            std::string v = urlDecode(pair.substr(eqPos + 1));
            params[k] = v;
        } else if (!pair.empty()) {
            params[urlDecode(pair)] = "";
        }
    }
    return params;
}

} // namespace JsonHelper
