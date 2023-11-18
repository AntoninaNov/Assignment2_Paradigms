// CaesarCipher.h
#ifndef CAESARCIPHER_H
#define CAESARCIPHER_H

#include <string>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()

using namespace std;

extern "C" {
// Assuming these functions are provided by libcaesar.dylib
char* encrypt(const char* text, int key);
char* decrypt(const char* text, int key);
}

class CaesarCipher {
public:
    static string encryptText(const string& text, int key) {
        char* encrypted = encrypt(text.c_str(), key);
        string result(encrypted);
        free(encrypted);
        return result;
    }

    static string decryptText(const string& text, int key) {
        char* decrypted = decrypt(text.c_str(), key);
        string result(decrypted);
        free(decrypted);
        return result;
    }

    static int generateRandomKey() {
        srand(time(NULL));
        return rand();
    }
};

#endif // CAESARCIPHER_H
