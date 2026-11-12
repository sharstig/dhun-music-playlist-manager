#ifndef SONG_NODE_H
#define SONG_NODE_H

#include <string>

/**
 * @brief SongNode represents a single song node in the Doubly Circular Linked List.
 * 
 * Each node stores song metadata and two pointers:
 * - next: points to the subsequent song in the circular playlist
 * - prev: points to the preceding song in the circular playlist
 */
struct SongNode {
    int id;                 // Unique identifier for the song
    std::string title;      // Song title
    std::string artist;     // Artist / Creator name
    int duration;           // Duration in seconds
    std::string coverUrl;   // Optional cover art URL or placeholder gradient key
    std::string audioUrl;   // Optional audio track URL or synth preset

    SongNode* next;         // Pointer to next song node
    SongNode* prev;         // Pointer to previous song node

    // Constructor to initialize a SongNode
    SongNode(int songId, const std::string& songTitle, const std::string& songArtist,
             int songDuration, const std::string& cover = "", const std::string& audio = "")
        : id(songId),
          title(songTitle),
          artist(songArtist),
          duration(songDuration),
          coverUrl(cover),
          audioUrl(audio),
          next(nullptr),
          prev(nullptr) {}
};

#endif // SONG_NODE_H
