#ifndef DOUBLY_CIRCULAR_LIST_H
#define DOUBLY_CIRCULAR_LIST_H

#include "SongNode.h"
#include <string>
#include <vector>

/**
 * @brief Plain struct used only for exporting a snapshot of song data
 * to JSON/REST API callers without exposing raw pointers.
 * The playlist itself is entirely managed by the DoublyCircularLinkedList pointers.
 */
struct SongDTO {
    int id;
    std::string title;
    std::string artist;
    int duration;
    std::string coverUrl;
    std::string audioUrl;
    int prevId;
    int nextId;
    bool isCurrent;
    bool isHead;
    bool isTail;
};

/**
 * @brief DoublyCircularLinkedList implements the playlist data structure.
 * 
 * Invariants:
 * - Empty list: head == nullptr, tail == nullptr, current == nullptr, songCount == 0.
 * - Single-node list: head == tail == current, head->next == head, head->prev == head.
 * - Multi-node list: head->prev == tail, tail->next == head.
 * - All forward traversals loop from tail back to head seamlessly.
 * - All backward traversals loop from head back to tail seamlessly.
 */
class DoublyCircularLinkedList {
private:
    SongNode* head;         // Pointer to the first song node
    SongNode* tail;         // Pointer to the last song node
    SongNode* current;      // Pointer to the currently playing song node
    int songCount;          // Total number of songs in the playlist
    int nextAutoId;         // Auto-increment ID generator for new songs

public:
    // Constructor
    DoublyCircularLinkedList();

    // Destructor (frees all allocated nodes safely)
    ~DoublyCircularLinkedList();

    // Disable copy constructor and assignment to prevent shallow copy issues
    DoublyCircularLinkedList(const DoublyCircularLinkedList&) = delete;
    DoublyCircularLinkedList& operator=(const DoublyCircularLinkedList&) = delete;

    // --- Core Operations ---

    /**
     * @brief Adds a new song to the end (tail) of the circular playlist.
     * @param title Title of the song.
     * @param artist Artist name.
     * @param duration Duration in seconds.
     * @param id Optional explicit ID (0 for auto-increment).
     * @param cover Optional cover URL or gradient identifier.
     * @param audio Optional audio URL or synth preset.
     * @return Pointer to newly created SongNode, or nullptr on failure (e.g., duplicate ID).
     */
    SongNode* addSong(const std::string& title, const std::string& artist, int duration,
                      int id = 0, const std::string& cover = "", const std::string& audio = "");

    /**
     * @brief Deletes a song by its unique ID.
     * @param id The ID to delete.
     * @return true if found and deleted, false otherwise.
     */
    bool deleteSongById(int id);

    /**
     * @brief Deletes a song by matching title (case-insensitive).
     * @param title The title to search and delete.
     * @return true if found and deleted, false otherwise.
     */
    bool deleteSongByTitle(const std::string& title);

    /**
     * @brief Searches for a song by unique ID.
     * @param id The ID to search.
     * @return Pointer to the SongNode if found, or nullptr.
     */
    SongNode* searchById(int id) const;

    /**
     * @brief Searches for a song by title (case-insensitive substring match).
     * @param query Search query string.
     * @return Pointer to the first matching SongNode, or nullptr.
     */
    SongNode* searchByTitle(const std::string& query) const;

    /**
     * @brief Searches for all songs matching title or artist query.
     * @param query Search query string.
     * @return Vector of SongDTOs representing matching songs.
     */
    std::vector<SongDTO> searchAll(const std::string& query) const;

    /**
     * @brief Navigates to the next song in the circular playlist.
     * If current is tail, circularly wraps around to head.
     * @return Pointer to new current song, or nullptr if empty.
     */
    SongNode* nextSong();

    /**
     * @brief Navigates to the previous song in the circular playlist.
     * If current is head, circularly wraps around to tail.
     * @return Pointer to new current song, or nullptr if empty.
     */
    SongNode* prevSong();

    /**
     * @brief Sets the current song pointer to the node with the given ID.
     * @param id The song ID to select.
     * @return Pointer to current song, or nullptr if not found.
     */
    SongNode* setCurrentSong(int id);

    // --- State & Getters ---

    SongNode* getHead() const { return head; }
    SongNode* getTail() const { return tail; }
    SongNode* getCurrent() const { return current; }
    int getSongCount() const { return songCount; }
    bool isEmpty() const { return songCount == 0 || head == nullptr; }

    /**
     * @brief Exports all songs in playlist order (from head to tail) as DTO snapshots.
     */
    std::vector<SongDTO> getAllSongs() const;

    /**
     * @brief Exports the currently playing song as a DTO snapshot.
     */
    bool getCurrentSongDTO(SongDTO& out) const;

    /**
     * @brief Verifies that the doubly circular invariant holds.
     * Used for unit testing and viva verification.
     */
    bool verifyInvariants() const;

    /**
     * @brief Deletes all songs and resets the playlist to empty state.
     */
    void clear();

    /**
     * @brief Seeds the playlist with initial default tracks for demonstration.
     */
    void seedDefaultSongs();
};

#endif // DOUBLY_CIRCULAR_LIST_H
