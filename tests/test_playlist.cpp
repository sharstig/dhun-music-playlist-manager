#include "DoublyCircularList.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// Color-coded test assertion macros
#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << " [FAIL] " << msg << " (Line " << __LINE__ << ")\n"; \
            return false; \
        } \
    } while(0)

bool testEmptyList() {
    std::cout << "[RUNNING] testEmptyList...\n";
    DoublyCircularLinkedList list;
    TEST_ASSERT(list.isEmpty(), "List should be initially empty");
    TEST_ASSERT(list.getSongCount() == 0, "Song count should be 0");
    TEST_ASSERT(list.getHead() == nullptr, "Head should be null");
    TEST_ASSERT(list.getTail() == nullptr, "Tail should be null");
    TEST_ASSERT(list.getCurrent() == nullptr, "Current should be null");
    TEST_ASSERT(list.nextSong() == nullptr, "nextSong on empty list should return null");
    TEST_ASSERT(list.prevSong() == nullptr, "prevSong on empty list should return null");
    TEST_ASSERT(!list.deleteSongById(999), "Deleting non-existent ID on empty list should return false");
    TEST_ASSERT(!list.deleteSongByTitle("Unknown"), "Deleting by title on empty list should return false");
    TEST_ASSERT(list.searchById(101) == nullptr, "Search by ID should return null");
    TEST_ASSERT(list.searchByTitle("Test") == nullptr, "Search by title should return null");
    TEST_ASSERT(list.verifyInvariants(), "Invariants should hold for empty list");
    std::cout << " [PASS] testEmptyList\n";
    return true;
}

bool testSingleNodeOperations() {
    std::cout << "[RUNNING] testSingleNodeOperations...\n";
    DoublyCircularLinkedList list;
    SongNode* s1 = list.addSong("Song One", "Artist A", 180, 101);
    TEST_ASSERT(s1 != nullptr, "addSong should return created node");
    TEST_ASSERT(list.getSongCount() == 1, "Song count should be 1");
    TEST_ASSERT(!list.isEmpty(), "List should not be empty");
    TEST_ASSERT(list.getHead() == s1, "Head should be s1");
    TEST_ASSERT(list.getTail() == s1, "Tail should be s1");
    TEST_ASSERT(list.getCurrent() == s1, "Current should be s1");

    // Circular invariants for 1 node: next and prev must point to itself!
    TEST_ASSERT(s1->next == s1, "s1->next must point to s1 (circular)");
    TEST_ASSERT(s1->prev == s1, "s1->prev must point to s1 (circular)");
    TEST_ASSERT(list.verifyInvariants(), "Invariants must hold for single node");

    // Navigation on single node
    TEST_ASSERT(list.nextSong() == s1, "nextSong on single node returns s1");
    TEST_ASSERT(list.prevSong() == s1, "prevSong on single node returns s1");

    // Deleting single node
    bool deleted = list.deleteSongById(101);
    TEST_ASSERT(deleted, "Deleting s1 by ID should succeed");
    TEST_ASSERT(list.isEmpty(), "List should be empty after deleting sole node");
    TEST_ASSERT(list.getHead() == nullptr, "Head should be null");
    TEST_ASSERT(list.getTail() == nullptr, "Tail should be null");
    TEST_ASSERT(list.getCurrent() == nullptr, "Current should be null");
    TEST_ASSERT(list.verifyInvariants(), "Invariants should hold after emptying list");

    std::cout << " [PASS] testSingleNodeOperations\n";
    return true;
}

bool testMultiNodeOperationsAndCircularTraversal() {
    std::cout << "[RUNNING] testMultiNodeOperationsAndCircularTraversal...\n";
    DoublyCircularLinkedList list;

    SongNode* s1 = list.addSong("Song A", "Artist 1", 120, 101);
    SongNode* s2 = list.addSong("Song B", "Artist 2", 140, 102);
    SongNode* s3 = list.addSong("Song C", "Artist 3", 160, 103);
    SongNode* s4 = list.addSong("Song D", "Artist 4", 180, 104);

    TEST_ASSERT(list.getSongCount() == 4, "Song count should be 4");
    TEST_ASSERT(list.getHead() == s1, "Head should be s1");
    TEST_ASSERT(list.getTail() == s4, "Tail should be s4");
    TEST_ASSERT(list.getCurrent() == s1, "Current should start at s1");

    // Verify circular linkages
    TEST_ASSERT(s4->next == s1, "tail->next must be head (s4->next == s1)");
    TEST_ASSERT(s1->prev == s4, "head->prev must be tail (s1->prev == s4)");
    TEST_ASSERT(s1->next == s2, "s1->next == s2");
    TEST_ASSERT(s2->next == s3, "s2->next == s3");
    TEST_ASSERT(s3->next == s4, "s3->next == s4");
    TEST_ASSERT(s4->prev == s3, "s4->prev == s3");
    TEST_ASSERT(s3->prev == s2, "s3->prev == s2");
    TEST_ASSERT(s2->prev == s1, "s2->prev == s1");
    TEST_ASSERT(list.verifyInvariants(), "Multi-node invariants must hold");

    // Test Forward Circular Traversal (Looping over 8 steps = 2 full cycles)
    // s1 -> s2 -> s3 -> s4 -> s1 -> s2 -> s3 -> s4 -> s1
    TEST_ASSERT(list.nextSong() == s2, "s1 next is s2");
    TEST_ASSERT(list.nextSong() == s3, "s2 next is s3");
    TEST_ASSERT(list.nextSong() == s4, "s3 next is s4");
    TEST_ASSERT(list.nextSong() == s1, "s4 next wraps circularly to s1");
    TEST_ASSERT(list.nextSong() == s2, "Second loop s1 next is s2");
    TEST_ASSERT(list.nextSong() == s3, "Second loop s2 next is s3");
    TEST_ASSERT(list.nextSong() == s4, "Second loop s3 next is s4");
    TEST_ASSERT(list.nextSong() == s1, "Second loop s4 next wraps to s1");

    // Test Backward Circular Traversal (Looping backwards from s1 to s4)
    // s1 -> s4 -> s3 -> s2 -> s1
    TEST_ASSERT(list.prevSong() == s4, "s1 prev wraps backwards circularly to s4 (tail)");
    TEST_ASSERT(list.prevSong() == s3, "s4 prev is s3");
    TEST_ASSERT(list.prevSong() == s2, "s3 prev is s2");
    TEST_ASSERT(list.prevSong() == s1, "s2 prev is s1");

    std::cout << " [PASS] testMultiNodeOperationsAndCircularTraversal\n";
    return true;
}

bool testDeletions() {
    std::cout << "[RUNNING] testDeletions (Head, Tail, Middle)...\n";
    DoublyCircularLinkedList list;

    list.addSong("Song A", "Artist 1", 100, 101);
    list.addSong("Song B", "Artist 2", 200, 102);
    list.addSong("Song C", "Artist 3", 300, 103);
    list.addSong("Song D", "Artist 4", 400, 104);
    list.addSong("Song E", "Artist 5", 500, 105);

    // 1. Delete Middle Node (103: Song C)
    bool delMiddle = list.deleteSongById(103);
    TEST_ASSERT(delMiddle, "Middle node 103 should be deleted");
    TEST_ASSERT(list.getSongCount() == 4, "Count should be 4");
    TEST_ASSERT(list.searchById(103) == nullptr, "103 should no longer exist");
    // Check neighbor re-linking (102 next should be 104, 104 prev should be 102)
    SongNode* n102 = list.searchById(102);
    SongNode* n104 = list.searchById(104);
    TEST_ASSERT(n102->next == n104, "102->next must now point to 104");
    TEST_ASSERT(n104->prev == n102, "104->prev must now point to 102");
    TEST_ASSERT(list.verifyInvariants(), "Invariants hold after middle deletion");

    // 2. Delete Head Node (101: Song A)
    bool delHead = list.deleteSongById(101);
    TEST_ASSERT(delHead, "Head node 101 should be deleted");
    TEST_ASSERT(list.getSongCount() == 3, "Count should be 3");
    TEST_ASSERT(list.getHead()->id == 102, "New head should be 102");
    TEST_ASSERT(list.getTail()->id == 105, "Tail should still be 105");
    TEST_ASSERT(list.getTail()->next == list.getHead(), "tail->next points to new head");
    TEST_ASSERT(list.getHead()->prev == list.getTail(), "new head->prev points to tail");
    TEST_ASSERT(list.verifyInvariants(), "Invariants hold after head deletion");

    // 3. Delete Tail Node (105: Song E)
    bool delTail = list.deleteSongById(105);
    TEST_ASSERT(delTail, "Tail node 105 should be deleted");
    TEST_ASSERT(list.getSongCount() == 2, "Count should be 2");
    TEST_ASSERT(list.getTail()->id == 104, "New tail should be 104");
    TEST_ASSERT(list.getTail()->next == list.getHead(), "new tail->next points to head (102)");
    TEST_ASSERT(list.getHead()->prev == list.getTail(), "head->prev points to new tail (104)");
    TEST_ASSERT(list.verifyInvariants(), "Invariants hold after tail deletion");

    // 4. Delete by Title (case-insensitive)
    bool delByTitle = list.deleteSongByTitle("song d");
    TEST_ASSERT(delByTitle, "Deleting by title 'song d' should delete 104");
    TEST_ASSERT(list.getSongCount() == 1, "Count should now be 1");
    TEST_ASSERT(list.getHead()->id == 102, "Remaining node is 102");
    TEST_ASSERT(list.getTail()->id == 102, "Tail is also 102");
    TEST_ASSERT(list.verifyInvariants(), "Invariants hold for 1 node");

    // 5. Delete final remaining node by title
    bool delLast = list.deleteSongByTitle("SONG B");
    TEST_ASSERT(delLast, "Deleting final node should succeed");
    TEST_ASSERT(list.isEmpty(), "List should be empty");
    TEST_ASSERT(list.verifyInvariants(), "Invariants hold for empty list");

    std::cout << " [PASS] testDeletions\n";
    return true;
}

bool testSearchAndValidation() {
    std::cout << "[RUNNING] testSearchAndValidation...\n";
    DoublyCircularLinkedList list;

    // Reject invalid inputs
    TEST_ASSERT(list.addSong("", "Artist", 100) == nullptr, "Empty title should be rejected");
    TEST_ASSERT(list.addSong("Title", "", 100) == nullptr, "Empty artist should be rejected");
    TEST_ASSERT(list.addSong("Title", "Artist", 0) == nullptr, "0 duration should be rejected");
    TEST_ASSERT(list.addSong("Title", "Artist", -10) == nullptr, "Negative duration should be rejected");

    // Add valid song
    SongNode* s1 = list.addSong("Chasing Shadows", "Luna", 200, 201);
    TEST_ASSERT(s1 != nullptr, "Valid song should be added");

    // Duplicate ID rejection
    SongNode* dup = list.addSong("Another Song", "Different Artist", 150, 201);
    TEST_ASSERT(dup == nullptr, "Duplicate ID 201 must be rejected");
    TEST_ASSERT(list.getSongCount() == 1, "Count should still be 1 after duplicate rejection");

    list.addSong("Neon Lights", "SynthWave", 180, 202);

    // Search tests
    TEST_ASSERT(list.searchById(201) == s1, "Search by ID 201 should find s1");
    TEST_ASSERT(list.searchById(999) == nullptr, "Search non-existent ID should return null");

    TEST_ASSERT(list.searchByTitle("shadows") != nullptr, "Substring search 'shadows' should find s1");
    TEST_ASSERT(list.searchByTitle("LIGHTS") != nullptr, "Case-insensitive search 'LIGHTS' should find s2");
    TEST_ASSERT(list.searchByTitle("NonExistent") == nullptr, "Non-existent title should return null");

    // SearchAll
    auto results = list.searchAll("wave");
    TEST_ASSERT(results.size() == 1, "searchAll('wave') should find 1 match (SynthWave)");
    TEST_ASSERT(results[0].id == 202, "Match should have ID 202");

    std::cout << " [PASS] testSearchAndValidation\n";
    return true;
}

int main() {
    std::cout << "\n=========================================\n";
    std::cout << "  DHUN - DSA UNIT TEST SUITE (C++17)     \n";
    std::cout << "  Doubly Circular Linked List Validation \n";
    std::cout << "=========================================\n\n";

    bool allPassed = true;
    allPassed &= testEmptyList();
    allPassed &= testSingleNodeOperations();
    allPassed &= testMultiNodeOperationsAndCircularTraversal();
    allPassed &= testDeletions();
    allPassed &= testSearchAndValidation();

    std::cout << "\n-----------------------------------------\n";
    if (allPassed) {
        std::cout << " ALL TESTS PASSED SUCCESSFULLY! (100% OK)\n";
        std::cout << "-----------------------------------------\n\n";
        return 0;
    } else {
        std::cerr << " SOME TESTS FAILED!\n";
        std::cout << "-----------------------------------------\n\n";
        return 1;
    }
}
