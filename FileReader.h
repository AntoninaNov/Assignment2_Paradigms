#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>
#include <vector>

using namespace std;

// IReader interface
class IReader {
public:
    virtual string read(const string& filePath) = 0;
    virtual ~IReader() = default;
};

// FileReader class implementing IReader
class FileReader : public IReader {
public:
    string read(const string& filePath) override {
        ifstream file(filePath, ios::in | ios::binary);
        if (!file.is_open()) {
            throw runtime_error("File not found: " + filePath);
        }

        const size_t chunkSize = 128;
        vector<char> buffer(chunkSize + 1, '\0');
        string content;

        while (!file.eof()) {
            file.read(buffer.data(), chunkSize);
            // In case the last chunk is smaller than chunkSize
            buffer[file.gcount()] = '\0';
            content += string(buffer.data());
        }

        file.close();
        return content;
    }
};

#endif // FILEREADER_H