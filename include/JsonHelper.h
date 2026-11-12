#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include "DoublyCircularList.h"
#include <string>
#include <vector>
#include <map>

namespace JsonHelper {

    // String escaping for JSON
    std::string escape(const std::string& input);

    // Serialization functions
    std::string songToJson(const SongDTO& song);
    std::string songsToJson(const std::vector<SongDTO>& songs, int currentId, int headId, int tailId, int count);
    std::string errorJson(const std::string& message, int statusCode = 400);
    std::string successJson(const std::string& message);
    std::string dsaStateJson(const DoublyCircularLinkedList& list);

    // Deserialization / Parsing
    // Extracts string value for a key from a JSON object
    bool getString(const std::string& json, const std::string& key, std::string& out);

    // Extracts integer value for a key from a JSON object
    bool getInt(const std::string& json, const std::string& key, int& out);

    // URL / Query string parser (e.g. "?id=101&title=foo")
    std::map<std::string, std::string> parseQueryString(const std::string& query);
    std::string urlDecode(const std::string& src);

} // namespace JsonHelper

#endif // JSON_HELPER_H
