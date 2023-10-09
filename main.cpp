#include <iostream>
#include <string>
#include <fstream>
#include <memory>

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

class TextList {
private:
    unique_ptr<TextNode> head;

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

        ofstream file(filename);
        if (file.is_open()) {
            for (TextNode* current = head.get(); current; current = current->next.get()) {
                file << current->content;
                if (current->next) {
                    file << '\n';
                }
            }
            file.close();
            cout << "Text has been saved successfully\n";
        } else {
            cout << "Error: Could not save to file!\n";
        }
    }

    void loadFromFile() {
        undoStack.pushState(head);
        redoStack.clear();
        string filename;
        cout << "Enter the file name for loading: ";
        getline(cin, filename);

        ifstream file(filename);
        if (file.is_open()) {
            head = make_unique<TextNode>(); // Clearing out the existing linked list

            TextNode* current = head.get();
            string line;
            bool isFirstLine = true;

            while (getline(file, line)) {
                if (isFirstLine) {
                    current->content = line;
                    isFirstLine = false;
                } else {
                    current->next = make_unique<TextNode>(line);
                    current = current->next.get();
                }
            }

            file.close();
            cout << "Text has been loaded successfully\n";
        } else {
            cout << "Error: Could not load from file!\n";
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
        head = undoStack.popState();
    }

    void redoLastChange() {
        if (redoStack.isEmpty()) {
            cout << "No more steps to redo!" << endl;
            return;
        }
        undoStack.pushState(head);  // Save current state for undo
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

        string textToReplace;
        cout << "Enter the text you want to replace: ";
        getline(cin, textToReplace);

        TextNode* targetNode = findTextNodeAtIndex(lineIndex);
        if (!targetNode) {
            cout << "Line index provided is out of bounds!" << endl;
            return;
        }

        size_t posToReplace = targetNode->content.find(textToReplace, charIndex);

        if (posToReplace != string::npos) {
            cout << "Enter new text to insert: ";
            string newText;
            getline(cin, newText);
            targetNode->content.replace(posToReplace, textToReplace.length(), newText);
        } else {
            cout << "Text to replace not found from the given index!" << endl;
        }
    }

};

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
    cout << "Your choice: ";
}

int main() {
    TextList list;
    char userCommand;

    while (true) {
        // Display menu at the beginning of each loop iteration
        displayMenu();

        cin >> userCommand;
        cin.ignore();  // Clear input buffer

        switch (userCommand) {
            case '1':
            {
                string textToAppend;
                cout << "Enter text to append: ";
                getline(cin, textToAppend);
                list.appendToEnd(textToAppend);
            }
                break;
            case '2':
                list.startNewLine();
                break;
            case '3':
                list.saveToFile();
                break;
            case '4':
                list.loadFromFile();
                break;
            case '5':
                list.printToConsole();
                break;
            case '6':
                list.insertTextByIndexes();
                break;
            case '7':
                list.searchTextInList();
                break;
            case '8':
                clearConsole();
                break;
            case '9':
                list.deleteTextByIndexes();
                break;
            case '10':
                cout << "Exiting program..." << endl;
                // Destructor of TextList will handle memory cleanup
                exit(0);
                break;
            case '11':
                list.undoLastChange();
                break;
            case '12':
                list.redoLastChange();
                break;
            case '13':
                list.cutTextByIndexes();
                break;
            case '14':
                list.copyTextByIndexes();
                break;
            case '15':
                list.pasteTextByIndexes();
                break;
            case '16':
                list.insertWithReplaceTextByIndexes();
                break;
            default:
                cout << "Invalid command, please enter a valid command." << endl;
                break;
        }
    }
    return 0;
}
