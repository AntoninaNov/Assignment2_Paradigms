#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>

using namespace std;

// IWriter interface
class IWriter {
public:
    virtual void write(const string& filePath, const string& text) = 0;
    virtual ~IWriter() = default;
};

class FileWriter : public IWriter {
public:
    void write(const string& filePath, const string& text) override {
        // Check if file exists
        ifstream fileTest(filePath);

        // Write the file in chunks
        ofstream file(filePath, ios::out | ios::binary);
        if (!file.is_open()) {
            throw runtime_error("Unable to open file: " + filePath);
        }

        const size_t chunkSize = 128;
        for (size_t i = 0; i < text.size(); i += chunkSize) {
            string chunk = text.substr(i, chunkSize);
            file.write(chunk.c_str(), chunk.size());
        }

        file.close();
    }
};

#endif // FILEWRITER_H
