#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <stdexcept>
#include "FileReader.h"
#include "FileWriter.h"
#include "CaesarCipher.h"

using namespace std;

class TextNode {
public:
    string content;
    unique_ptr<TextNode> next;

    TextNode(const string &content = "") : content(content), next(nullptr) {}
};


class HistoryStack {
private:
    stack<unique_ptr<TextNode>> history;
    const int maxSteps;

public:
    HistoryStack(int steps) : maxSteps(steps) {}

    void pushState(const unique_ptr<TextNode>& head) {
        if (history.size() == maxSteps) {
            history.pop();
        }
        unique_ptr<TextNode> clonedHead = cloneList(head);
        history.push(move(clonedHead));
    }

    unique_ptr<TextNode> popState() {
        if (history.empty()) return nullptr;
        unique_ptr<TextNode> lastState = move(history.top());
        history.pop();
        return lastState;
    }

    bool isEmpty() const {
        return history.empty();
    }

    void clear() {
        while (!history.empty()) {
            history.pop();
        }
    }

private:
    static unique_ptr<TextNode> cloneList(const unique_ptr<TextNode>& head) {
        if (!head) return nullptr;
        unique_ptr<TextNode> clonedHead = make_unique<TextNode>(head->content);
        TextNode* currentSrc = head.get();
        TextNode* currentDst = clonedHead.get();
        while (currentSrc->next) {
            currentDst->next = make_unique<TextNode>(currentSrc->next->content);
            currentSrc = currentSrc->next.get();
            currentDst = currentDst->next.get();
        }
        return clonedHead;
    }
};

class Cursor {
public:
    int lineIndex;
    int charIndex;

    Cursor() : lineIndex(0), charIndex(0) {}
};

class TextList {
private:
    unique_ptr<TextNode> head;
    Cursor cursor;

    TextNode* findLastTextNode() {
        TextNode* current = head.get();
        while (current->next) {
            current = current->next.get();
        }
        return current;
    }

    HistoryStack undoStack{3};
    HistoryStack redoStack{3};
    string clipboardBuffer; // store copied/cut text

public:
    TextList() : head(make_unique<TextNode>()) {}

    void setCursor(int line, int pos) {
        cursor.lineIndex = line;
        cursor.charIndex = pos;
    }

    void printCursorPosition() {
        cout << "Cursor is at line " << cursor.lineIndex << ", position " << cursor.charIndex << endl;
    }
    void appendToEnd(const string &textToAppend) {
        undoStack.pushState(head);
        redoStack.clear();
        TextNode* lastNode = findLastTextNode();
        if (lastNode->content.empty()) {
            lastNode->content = textToAppend;
        } else {
            lastNode->next = make_unique<TextNode>(textToAppend);
        }
    }

    void startNewLine() {
        undoStack.pushState(head);
        redoStack.clear();
        TextNode* lastNode = findLastTextNode();
        lastNode->next = make_unique<TextNode>();
    }

    void saveToFile() {
        string filename;
        cout << "Enter the file name for saving: ";
        getline(cin, filename);

        FileWriter writer; // Create an instance of FileWriter

        try {
            stringstream fileContent;
            for (TextNode* current = head.get(); current; current = current->next.get()) {
                fileContent << current->content;
                if (current->next) {
                    fileContent << '\n';
                }
            }
            writer.write(filename, fileContent.str()); // Use FileWriter to write to file
            cout << "Text has been saved successfully\n";
        } catch (const runtime_error& e) {
            cerr << "Error: " << e.what() << endl;
        }
    }
    void loadFromFile() {
        undoStack.pushState(head);
        redoStack.clear();

        string filename;
        cout << "Enter the file name for loading: ";
        getline(cin, filename);

        try {
            FileReader reader;
            string fileContent = reader.read(filename);

            head = make_unique<TextNode>(); // Clearing out the existing linked list

            TextNode* current = head.get();
            stringstream ss(fileContent);
            string line;
            bool isFirstLine = true;

            while (getline(ss, line)) {
                if (isFirstLine) {
                    current->content = line;
                    isFirstLine = false;
                } else {
                    current->next = make_unique<TextNode>(line);
                    current = current->next.get();
                }
            }

            cout << "Text has been loaded successfully\n";
        } catch (const runtime_error& e) {
            cerr << "Error: " << e.what() << endl;
        }
    }


    TextNode* findTextNodeAtIndex(int index) {
        int currentIndex = 0;
        TextNode* current = head.get();
        while (current && currentIndex != index) {
            current = current->next.get();
            currentIndex++;
        }
        return current;
    }

    void insertTextByIndexes() {
        undoStack.pushState(head);
        redoStack.clear();
        int lineIndex, charIndex;
        cout << "Enter line index and character index separated by space: ";
        cin >> lineIndex >> charIndex;
        cin.ignore();  // Clear input buffer

        cout << "Enter text to insert: ";
        string insertText;
        getline(cin, insertText);

        TextNode* targetNode = findTextNodeAtIndex(lineIndex);
        if (!targetNode || charIndex > targetNode->content.size() || charIndex < 0) {
            cout << "Invalid index provided!\n";
            return;
        }

        targetNode->content.insert(charIndex, insertText);
    }

    void searchTextInList() {
        cout << "Enter text to search: ";
        string searchText;
        getline(cin, searchText);

        TextNode* currentNode = head.get();
        bool isFound = false;
        int lineNumber = 0;

        while (currentNode) {
            size_t position = currentNode->content.find(searchText);
            while (position != string::npos) {
                cout << "Found on line " << lineNumber << " at position " << position << ": " << currentNode->content << "\n";
                isFound = true;

                // Search for the next occurrence in the same line
                position = currentNode->content.find(searchText, position + 1);
            }

            lineNumber++;
            currentNode = currentNode->next.get();
        }

        if (!isFound) {
            cout << "Text not found!\n";
        }
    }

    void printToConsole() {
        for (TextNode* current = head.get(); current; current = current->next.get()) {
            cout << current->content << '\n';
        }
    }

    void deleteTextByIndexes() {
        undoStack.pushState(head);
        redoStack.clear();
        int lineIndex, charIndex, numSymbols;
        cout << "Enter line index, character index, and number of symbols to delete separated by space: ";
        cin >> lineIndex >> charIndex >> numSymbols;
        cin.ignore();  // Clear input buffer

        TextNode* targetNode = findTextNodeAtIndex(lineIndex);
        if (!targetNode || charIndex >= targetNode->content.size() || charIndex < 0) {
            cout << "Invalid index provided!\n";
            return;
        }

        // Ensure we do not exceed the string's size
        if(charIndex + numSymbols > targetNode->content.size()) {
            numSymbols = targetNode->content.size() - charIndex;
        }

        targetNode->content.erase(charIndex, numSymbols);
    }

    void undoLastChange() {
        if (undoStack.isEmpty()) {
            cout << "No more steps to undo!" << endl;
            return;
        }
        // Push the current state to the redo stack before undoing
        redoStack.pushState(head);
        // Then pop the last state from the undo stack
        head = undoStack.popState();
    }

    void redoLastChange() {
        if (redoStack.isEmpty()) {
            cout << "No more steps to redo!" << endl;
            return;
        }
        // Save the current state to undo stack before redoing
        undoStack.pushState(head);
        // Then pop the last state from the redo stack
        head = redoStack.popState();
    }


    void cutTextByIndexes() {
        undoStack.pushState(head);
        redoStack.clear();

        int lineIndex, charIndex, numSymbols;
        cout << "Enter line index, character index, and number of symbols to cut separated by space: ";
        cin >> lineIndex >> charIndex >> numSymbols;
        cin.ignore();

        TextNode* targetNode = findTextNodeAtIndex(lineIndex);
        if (!targetNode || charIndex >= targetNode->content.size() || charIndex < 0) {
            cout << "Invalid index provided!\n";
            return;
        }

        // Ensure we do not exceed the string's size
        if(charIndex + numSymbols > targetNode->content.size()) {
            numSymbols = targetNode->content.size() - charIndex;
        }

        clipboardBuffer = targetNode->content.substr(charIndex, numSymbols);
        targetNode->content.erase(charIndex, numSymbols);
    }

    void copyTextByIndexes() {
        int lineIndex, charIndex, numSymbols;
        cout << "Enter line index, character index, and number of symbols to copy separated by space: ";
        cin >> lineIndex >> charIndex >> numSymbols;
        cin.ignore();

        TextNode* targetNode = findTextNodeAtIndex(lineIndex);
        if (!targetNode || charIndex >= targetNode->content.size() || charIndex < 0) {
            cout << "Invalid index provided!\n";
            return;
        }

        // Ensure we do not exceed the string's size
        if(charIndex + numSymbols > targetNode->content.size()) {
            numSymbols = targetNode->content.size() - charIndex;
        }

        clipboardBuffer = targetNode->content.substr(charIndex, numSymbols);
    }

    void pasteTextByIndexes() {
        undoStack.pushState(head);
        redoStack.clear();

        int lineIndex, charIndex;
        cout << "Enter line index and character index separated by space: ";
        cin >> lineIndex >> charIndex;
        cin.ignore();

        TextNode* targetNode = findTextNodeAtIndex(lineIndex);
        if (!targetNode || charIndex > targetNode->content.size() || charIndex < 0) {
            cout << "Invalid index provided!\n";
            return;
        }

        targetNode->content.insert(charIndex, clipboardBuffer);
    }

    void insertWithReplaceTextByIndexes() {
        undoStack.pushState(head);
        redoStack.clear();

        int lineIndex, charIndex;
        cout << "Enter line index and character index separated by space: ";
        cin >> lineIndex >> charIndex;
        cin.ignore();  // Clear input buffer

        TextNode* targetNode = findTextNodeAtIndex(lineIndex);
        if (!targetNode) {
            cout << "Line index provided is out of bounds!" << endl;
            return;
        }

        if (charIndex < 0 || charIndex >= targetNode->content.size()) {
            cout << "Invalid character index provided!" << endl;
            return;
        }

        cout << "Enter new text to insert: ";
        string newText;
        getline(cin, newText);

        targetNode->content.replace(charIndex, newText.length(), newText);
    }

};

void handleNormalMode() {
    cout << "Choose operation (1 for Encrypt, 2 for Decrypt): ";
    int operation;
    cin >> operation;
    cin.ignore();

    string inputPath, outputPath;
    int key;
    cout << "Enter input file path: ";
    getline(cin, inputPath);
    cout << "Enter output file path: ";
    getline(cin, outputPath);
    cout << "Enter key: ";
    cin >> key;
    cin.ignore();

    FileReader reader;
    FileWriter writer;
    string content = reader.read(inputPath);
    string result = (operation == 1) ? CaesarCipher::encryptText(content, key) : CaesarCipher::decryptText(content, key);
    writer.write(outputPath, result);
}

void handleSecretMode() {
    string inputPath, outputPath;
    cout << "Enter input file path: ";
    getline(cin, inputPath);
    cout << "Enter output file path: ";
    getline(cin, outputPath);

    int key = CaesarCipher::generateRandomKey();
    cout << "Generated key (for your record): " << key << endl;

    FileReader reader;
    FileWriter writer;
    string content = reader.read(inputPath);
    string result = CaesarCipher::encryptText(content, key);
    writer.write(outputPath, result);
}

void clearConsole() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    cout << "Console cleared" << endl;
}

void displayMenu() {
    cout << "Please select an option from the menu below:" << endl;
    cout << " 1 - Append at the end" << endl;
    cout << " 2 - Start a new line" << endl;
    cout << " 3 - Save to file" << endl;
    cout << " 4 - Load from file" << endl;
    cout << " 5 - Print to console" << endl;
    cout << " 6 - Insert text by line and index" << endl;
    cout << " 7 - Search" << endl;
    cout << " 8 - Clear console and display menu" << endl;
    cout << "9 - Delete text by line and index" << endl; // New option
    cout << "10 - Exit" << endl;
    cout << "11 - Undo last change" << endl;
    cout << "12 - Redo last undone change" << endl;
    cout << "13 - Cut text by line and index" << endl;
    cout << "14 - Copy text by line and index" << endl;
    cout << "15 - Paste text by line and index" << endl;
    cout << "16 - Insert with replacement by line and index" << endl;
    cout << "17 - Encrypt/Decrypt file (Normal Mode)" << endl;
    cout << "18 - Encrypt file (Secret Mode)" << endl;
    cout << "Your choice: ";
}

int main() {
    TextList list;
    int userCommand;

    while (true) {
        // Display menu at the beginning of each loop iteration
        displayMenu();

        cin >> userCommand;
        cin.ignore();  // Clear input buffer

        switch (userCommand) {
            case 1:
            {
                string textToAppend;
                cout << "Enter text to append: ";
                getline(cin, textToAppend);
                list.appendToEnd(textToAppend);
            }
                break;
            case 2:
                list.startNewLine();
                break;
            case 3:
                list.saveToFile();
                break;
            case 4:
                list.loadFromFile();
                break;
            case 5:
                list.printToConsole();
                break;
            case 6:
                list.insertTextByIndexes();
                break;
            case 7:
                list.searchTextInList();
                break;
            case 8:
                clearConsole();
                break;
            case 9:
                list.deleteTextByIndexes();
                break;
            case 10:
                cout << "Exiting program..." << endl;
                // Destructor of TextList will handle memory cleanup
                exit(0);
                break;
            case 11:
                list.undoLastChange();
                break;
            case 12:
                list.redoLastChange();
                break;
            case 13:
                list.cutTextByIndexes();
                break;
            case 14:
                list.copyTextByIndexes();
                break;
            case 15:
                list.pasteTextByIndexes();
                break;
            case 16:
                list.insertWithReplaceTextByIndexes();
                break;
            case 17:
                handleNormalMode();
                break;
            case 18:
                handleSecretMode();
                break;
            default:
                cout << "Invalid command, please enter a valid command." << endl;
                break;
        }
        cout << "\nCommand executed.\n";
    }
    return 0;
}
// /Users/antoninanovak/CLionProjects/Assignment2_Paradigms/cmake-build-debug/11_3.txt