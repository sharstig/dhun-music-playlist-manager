#include "DoublyCircularList.h"
#include <algorithm>
#include <cctype>
#include <iostream>

// Helper for case-insensitive string comparison / substring search
static std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower;
}

DoublyCircularLinkedList::DoublyCircularLinkedList()
    : head(nullptr), tail(nullptr), current(nullptr), songCount(0), nextAutoId(101) {}

DoublyCircularLinkedList::~DoublyCircularLinkedList() {
    clear();
}

void DoublyCircularLinkedList::clear() {
    if (head == nullptr) {
        tail = nullptr;
        current = nullptr;
        songCount = 0;
        return;
    }

    // Break circular link before deallocation to prevent infinite loop
    if (tail != nullptr) {
        tail->next = nullptr;
    }

    SongNode* curr = head;
    while (curr != nullptr) {
        SongNode* nextNode = curr->next;
        delete curr;
        curr = nextNode;
    }

    head = nullptr;
    tail = nullptr;
    current = nullptr;
    songCount = 0;
}

SongNode* DoublyCircularLinkedList::addSong(const std::string& title, const std::string& artist,
                                           int duration, int id, const std::string& cover,
                                           const std::string& audio) {
    if (title.empty() || artist.empty() || duration <= 0) {
        return nullptr; // Invalid input validation
    }

    // Determine ID
    int songId = id;
    if (songId > 0) {
        // Explicit ID provided: check for uniqueness
        if (searchById(songId) != nullptr) {
            return nullptr; // Duplicate ID rejected
        }
        if (songId >= nextAutoId) {
            nextAutoId = songId + 1;
        }
    } else {
        // Auto-increment ID
        songId = nextAutoId++;
    }

    // Create new node
    SongNode* newNode = new SongNode(songId, title, artist, duration, cover, audio);

    // Case 1: Empty list
    if (head == nullptr) {
        head = newNode;
        tail = newNode;
        newNode->next = newNode;
        newNode->prev = newNode;
        current = newNode; // First song automatically becomes current
        songCount = 1;
        return newNode;
    }

    // Case 2: Non-empty list (append to tail while maintaining circular connections)
    newNode->prev = tail;
    newNode->next = head;

    tail->next = newNode;
    head->prev = newNode;

    tail = newNode;
    songCount++;

    if (current == nullptr) {
        current = head;
    }

    return newNode;
}

bool DoublyCircularLinkedList::deleteSongById(int id) {
    if (isEmpty()) {
        return false;
    }

    SongNode* target = searchById(id);
    if (target == nullptr) {
        return false;
    }

    // Case 1: Single node in list
    if (songCount == 1) {
        delete target;
        head = nullptr;
        tail = nullptr;
        current = nullptr;
        songCount = 0;
        return true;
    }

    // Case 2: Multi-node list
    // Re-link neighbors
    target->prev->next = target->next;
    target->next->prev = target->prev;

    // Adjust head if target was head
    if (target == head) {
        head = target->next;
    }

    // Adjust tail if target was tail
    if (target == tail) {
        tail = target->prev;
    }

    // Adjust current if target was current playing song
    if (target == current) {
        current = target->next; // Smoothly advance to the next song
    }

    delete target;
    songCount--;
    return true;
}

bool DoublyCircularLinkedList::deleteSongByTitle(const std::string& title) {
    if (isEmpty() || title.empty()) {
        return false;
    }

    std::string searchTarget = toLower(title);
    SongNode* curr = head;

    for (int i = 0; i < songCount; ++i) {
        if (toLower(curr->title) == searchTarget) {
            return deleteSongById(curr->id);
        }
        curr = curr->next;
    }

    return false;
}

SongNode* DoublyCircularLinkedList::searchById(int id) const {
    if (isEmpty()) {
        return nullptr;
    }

    SongNode* curr = head;
    for (int i = 0; i < songCount; ++i) {
        if (curr->id == id) {
            return curr;
        }
        curr = curr->next;
    }

    return nullptr;
}

SongNode* DoublyCircularLinkedList::searchByTitle(const std::string& query) const {
    if (isEmpty() || query.empty()) {
        return nullptr;
    }

    std::string lowerQuery = toLower(query);
    SongNode* curr = head;

    for (int i = 0; i < songCount; ++i) {
        if (toLower(curr->title).find(lowerQuery) != std::string::npos) {
            return curr;
        }
        curr = curr->next;
    }

    return nullptr;
}

std::vector<SongDTO> DoublyCircularLinkedList::searchAll(const std::string& query) const {
    std::vector<SongDTO> results;
    if (isEmpty() || query.empty()) {
        return results;
    }

    std::string lowerQuery = toLower(query);
    SongNode* curr = head;

    for (int i = 0; i < songCount; ++i) {
        std::string titleLower = toLower(curr->title);
        std::string artistLower = toLower(curr->artist);
        std::string idStr = std::to_string(curr->id);

        if (titleLower.find(lowerQuery) != std::string::npos ||
            artistLower.find(lowerQuery) != std::string::npos ||
            idStr == query) {
            
            SongDTO dto;
            dto.id = curr->id;
            dto.title = curr->title;
            dto.artist = curr->artist;
            dto.duration = curr->duration;
            dto.coverUrl = curr->coverUrl;
            dto.audioUrl = curr->audioUrl;
            dto.prevId = curr->prev ? curr->prev->id : -1;
            dto.nextId = curr->next ? curr->next->id : -1;
            dto.isCurrent = (curr == current);
            dto.isHead = (curr == head);
            dto.isTail = (curr == tail);
            results.push_back(dto);
        }
        curr = curr->next;
    }

    return results;
}

SongNode* DoublyCircularLinkedList::nextSong() {
    if (isEmpty()) {
        return nullptr;
    }

    if (current == nullptr) {
        current = head;
        return current;
    }

    // Circular next traversal: if current was tail, current->next seamlessly points to head!
    current = current->next;
    return current;
}

SongNode* DoublyCircularLinkedList::prevSong() {
    if (isEmpty()) {
        return nullptr;
    }

    if (current == nullptr) {
        current = tail;
        return current;
    }

    // Circular prev traversal: if current was head, current->prev seamlessly points to tail!
    current = current->prev;
    return current;
}

SongNode* DoublyCircularLinkedList::setCurrentSong(int id) {
    SongNode* target = searchById(id);
    if (target != nullptr) {
        current = target;
        return current;
    }
    return nullptr;
}

std::vector<SongDTO> DoublyCircularLinkedList::getAllSongs() const {
    std::vector<SongDTO> list;
    if (isEmpty()) {
        return list;
    }

    list.reserve(songCount);
    SongNode* curr = head;

    for (int i = 0; i < songCount; ++i) {
        SongDTO dto;
        dto.id = curr->id;
        dto.title = curr->title;
        dto.artist = curr->artist;
        dto.duration = curr->duration;
        dto.coverUrl = curr->coverUrl;
        dto.audioUrl = curr->audioUrl;
        dto.prevId = curr->prev ? curr->prev->id : -1;
        dto.nextId = curr->next ? curr->next->id : -1;
        dto.isCurrent = (curr == current);
        dto.isHead = (curr == head);
        dto.isTail = (curr == tail);

        list.push_back(dto);
        curr = curr->next;
    }

    return list;
}

bool DoublyCircularLinkedList::getCurrentSongDTO(SongDTO& out) const {
    if (isEmpty() || current == nullptr) {
        return false;
    }

    out.id = current->id;
    out.title = current->title;
    out.artist = current->artist;
    out.duration = current->duration;
    out.coverUrl = current->coverUrl;
    out.audioUrl = current->audioUrl;
    out.prevId = current->prev ? current->prev->id : -1;
    out.nextId = current->next ? current->next->id : -1;
    out.isCurrent = true;
    out.isHead = (current == head);
    out.isTail = (current == tail);
    return true;
}

bool DoublyCircularLinkedList::verifyInvariants() const {
    if (songCount == 0) {
        return head == nullptr && tail == nullptr && current == nullptr;
    }

    if (songCount == 1) {
        return head != nullptr &&
               tail == head &&
               head->next == head &&
               head->prev == head &&
               current == head;
    }

    // In multi-node circular list:
    if (head == nullptr || tail == nullptr) return false;
    if (tail->next != head) return false;
    if (head->prev != tail) return false;

    // Traverse forward exactly songCount times and ensure we return to head
    SongNode* curr = head;
    for (int i = 0; i < songCount; ++i) {
        if (curr->next == nullptr || curr->prev == nullptr) return false;
        if (curr->next->prev != curr) return false; // Invariant: node->next->prev == node
        if (curr->prev->next != curr) return false; // Invariant: node->prev->next == node
        curr = curr->next;
    }

    return (curr == head);
}

void DoublyCircularLinkedList::seedDefaultSongs() {
    clear();
    // Default playlist for demonstration and viva presentation
    addSong("Midnight City Horizon", "Aura Synth", 214, 101, "sunset", "synth-lofi-1");
    addSong("Neon Pulse", "Cyberwave", 188, 102, "neon", "synth-lofi-2");
    addSong("Echoes of Bangalore", "Raga Fusion", 245, 103, "monsoon", "synth-lofi-3");
    addSong("Quantum Drifting", "Starlight", 192, 104, "aurora", "synth-lofi-4");
    addSong("Cosmic Coffee", "Chill Beats Collective", 168, 105, "coffee", "synth-lofi-5");
    addSong("Binary Sunset", "AlgoRhythm", 220, 106, "galaxy", "synth-lofi-6");
}
