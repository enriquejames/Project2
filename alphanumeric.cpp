#include <iostream>
#include <pthread.h>
#include <sstream>
#include <queue>
#include <atomic>
#include <condition_variable>

using namespace std;

atomic_flag lockFlag = ATOMIC_FLAG_INIT;  // Spinlock using atomic flag
condition_variable cv;  // Condition variable for signaling
queue<string> wordsQueue;  //  queue to store words
bool done = false;  // Flag to indicate completion of reading (program wasnt terminating in my terminal without this)

// Custom spinlock acquire function
void acquireLock() {
    while (lockFlag.test_and_set(std::memory_order_acquire)) {
        // Busy-wait loop
    }
}

// Custom spinlock release function
void releaseLock() {
    lockFlag.clear(std::memory_order_release);
}

// Function to check if a word starts with an alphabet character
bool startsWithAlpha(const string& word) {
    return !word.empty() && isalpha(word[0]);
}

// Function to check if a word starts with a numeric character
bool startsWithNumeric(const string& word) {
    return !word.empty() && isdigit(word[0]);
}

// Function to process alphabetic words
void* processAlphaWords(void* arg) {
    while (true) {
        acquireLock();
        if (wordsQueue.empty()) {
            releaseLock();
            if (done) break;  // Exit if reading is finished
            else continue;
        }

        if (!startsWithAlpha(wordsQueue.front())) {
            releaseLock();
            continue;
        }

        cout << "alpha: " << wordsQueue.front() << endl;
        wordsQueue.pop();
        releaseLock();
    }

    pthread_exit(NULL);
}

// Function to process numeric words
void* processNumericWords(void* arg) {
    while (true) {
        acquireLock();
        if (wordsQueue.empty()) {
            releaseLock();
            if (done) break;  // Exit if reading is finished
            else continue;
        }

        if (!startsWithNumeric(wordsQueue.front())) {
            releaseLock();
            continue;
        }

        cout << "numeric: " << wordsQueue.front() << endl;
        wordsQueue.pop();
        releaseLock();
    }

    pthread_exit(NULL);
}

int main() {
    cout << "Enter String: ";
    string inputPhrase;
    getline(cin, inputPhrase);
    cout << "\n";

    istringstream iss(inputPhrase);
    string word;

    pthread_t p1, p2;
    pthread_create(&p1, NULL, processAlphaWords, NULL);
    pthread_create(&p2, NULL, processNumericWords, NULL);

    while (iss >> word) {
        acquireLock();
        wordsQueue.push(word);
        releaseLock();
    }

    // Mark the end of reading
    acquireLock();
    done = true;
    releaseLock();

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    return 0;
}
