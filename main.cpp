#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>

using namespace std;

class CustomVector {
    char* data;
    size_t size;
    size_t capacity;

    void copyFrom(const CustomVector& other) {
        if (other.data) {
            data = new char[other.capacity];
            copy(other.data, other.data + other.size, data);
        }
        size = other.size;
        capacity = other.capacity;
    }

    void clearAll() {
        delete[] data;
        data = nullptr;
        size = 0;
        capacity = 0;
    }
public:
    CustomVector() : data(nullptr), size(0), capacity(0) {}

    CustomVector(const CustomVector& other) : data(nullptr), size(0), capacity(0) {
        copyFrom(other);
    }

    CustomVector& operator=(const CustomVector& other) {
        if (this != &other) {
            clearAll();
            copyFrom(other);
        }
        return *this;
    }

    ~CustomVector() {
        clearAll();
    }

    void push_back(char c) {
        if (size == capacity) {
            size_t newCapacity = (capacity == 0) ? 1 : capacity * 2;
            char* newData = new char[newCapacity];
            if (data) {
                for (size_t i = 0; i < size; ++i) {
                    newData[i] = data[i];
                }
                delete[] data;
            }
            data = newData;
            capacity = newCapacity;
        }
        data[size++] = c;
    }

    void resize(size_t newSize) {
        if (newSize < size) {
            size = newSize;
        }
    }

    void clear() {
        size = 0;
    }

    size_t getSize() const {
        return size;
    }

    char* getData() const {
        return data;
    }

    char& operator[](size_t index) {
        if (index >= size) {
            throw out_of_range("Index out of range");
        }
        return data[index];
    }

    const char& operator[](size_t index) const {
        if (index >= size) {
            throw out_of_range("Index out of range");
        }
        return data[index];
    }

    friend ostream& operator<<(ostream& os, const CustomVector& vec) {
        for (size_t i = 0; i < vec.size; ++i) {
            os << vec.data[i];
        }
        return os;
    }

    class Iterator {
    public:
        char* ptr;

        Iterator(char* p) : ptr(p) {}

        char& operator*() const {
            return *ptr;
        }

        Iterator& operator++() {
            ++ptr;
            return *this;
        }

        Iterator operator+(size_t offset) const {
            return {ptr + offset};
        }

        bool operator==(const Iterator& other) const {
            return ptr == other.ptr;
        }

        bool operator!=(const Iterator& other) const {
            return ptr != other.ptr;
        }
    };

    Iterator begin() {
        return {data};
    }

    Iterator end() {
        return {data + size};
    }

    void insert(Iterator pos, char value) {
        size_t index = pos.ptr - data;
        if (index > size) {
            throw out_of_range("Invalid position");
        }
        if (size == capacity) {
            size_t newCapacity = (capacity == 0) ? 1 : capacity * 2;
            char* newData = new char[newCapacity];
            for (size_t i = 0; i < index; ++i) {
                newData[i] = data[i];
            }
            newData[index] = value;
            for (size_t i = index; i < size; ++i) {
                newData[i + 1] = data[i];
            }
            delete[] data;
            data = newData;
            capacity = newCapacity;
            ++size;
        } else {
            for (size_t i = size; i > index; --i) {
                data[i] = data[i - 1];
            }
            data[index] = value;
            ++size;
        }
    }
};

class TextSource {
public:
    TextSource() = default;

    virtual void readData() = 0;
    virtual char* getData() = 0;
};

class TextFileSource : public TextSource {
    const char* fileName;
    CustomVector buffer;
public:
    TextFileSource() = delete;
    explicit TextFileSource(const char* fileName) : TextSource(), fileName(fileName) {};

    void readData() override {
        ifstream inputFile(fileName);

        if (!inputFile) {
            cerr << "Failed to open the file." << endl;
            return;
        }

        char inputChar;
        while (inputFile.get(inputChar)) {
            buffer.push_back(inputChar);
        }

        if (buffer.getSize() > 0) {
            buffer.push_back('\0');
        }
    }

    char* getData() override {
        return buffer.getData();
    }
};

class TextConsoleSource : public TextSource {
    CustomVector data;
public:
    TextConsoleSource() : TextSource() {};

    void readData() override {
        char inputChar;

        cout << "Enter characters: ";

        data.clear();

        while (cin.get(inputChar) && inputChar != '\n') {
            data.push_back(inputChar);
        }
        data.push_back('\0');
    }

    char* getData() override {
        return data.getData();
    }
};

class TextTransform {
protected:
    static const int maxLines = 1000;
    static const int maxLineLen = 1000;
public:
    explicit TextTransform() = default;

    virtual void apply(CustomVector& data) = 0;
};

class RemoveString : public TextTransform {
    const char* strToRemove;
public:
    explicit RemoveString(const char* strToRemove) : strToRemove(strToRemove) {}

    void apply(CustomVector& data) override {
        CustomVector result;
        char* read = data.getData();

        while (*read) {
            char* match = strstr(read, strToRemove);

            if (match) {
                size_t length = match - read;
                for (size_t i = 0; i < length; ++i) {
                    result.push_back(read[i]);
                }
                read = match + strlen(strToRemove);
            } else {
                while (*read) {
                    result.push_back(*read);
                    read++;
                }
                break;
            }
        }
        data.clear();

        for (size_t i = 0; i < result.getSize(); ++i) {
            data.push_back(result[i]);
        }
    }
};

class RemoveLines : public TextTransform {
    const char* substring;
public:
    explicit RemoveLines(const char* substring) : substring(substring) {}

    static void appendString(CustomVector& dest, const char* str) {
        for (size_t i = 0; str[i] != '\0'; ++i) {
            dest.push_back(str[i]);
        }
    }

    void apply(CustomVector& data) override {
        int numLines = 0;
        CustomVector result;

        char* token = strtok(data.getData(), "\n");

        while (token != nullptr && numLines < maxLines) {
            if (strstr(token, substring) == nullptr) {
                appendString(result, token);
                result.push_back('\n');
                numLines++;
            }
            token = strtok(nullptr, "\n");
        }
        data.clear();
        data = result;
    }
};

class RemoveCharacter : public TextTransform {
    const char charToRemove;
public:
    explicit RemoveCharacter(const char charToRemove) : charToRemove(charToRemove) {}

    void apply(CustomVector& data) override {
        size_t read = 0;
        size_t write = 0;

        while (read < data.getSize()) {
            if (data[read] != charToRemove) {
                data[write] = data[read];
                ++write;
            }
            ++read;
        }
        data.resize(write);
    }
};

class ReplaceString : public TextTransform {
    const char* oldStr;
    const char* newStr;
public:
    explicit ReplaceString(const char* oldStr, const char* newStr) : oldStr(oldStr), newStr(newStr) {}

    void apply(CustomVector& data) override {
        if (!oldStr || !newStr) {
            return;
        }
        size_t oldStrLen = strlen(oldStr);
        size_t newStrLen = strlen(newStr);
        size_t dataSize = data.getSize();

        CustomVector result;

        for (size_t i = 0; i < dataSize; ++i) {
            if (i + oldStrLen <= dataSize) {
                bool isMatch = true;
                for (size_t j = 0; j < oldStrLen; ++j) {
                    if (data[i + j] != oldStr[j]) {
                        isMatch = false;
                        break;
                    }
                }
                if (isMatch) {
                    for (size_t j = 0; j < newStrLen; ++j) {
                        result.push_back(newStr[j]);
                    }
                    i += oldStrLen - 1;
                } else {
                    result.push_back(data[i]);
                }
            } else {
                result.push_back(data[i]);
            }
        }
        data.clear();

        for (size_t i = 0; i < result.getSize(); ++i) {
            data.push_back(result[i]);
        }
    }
};

class RemovePunctuation : public TextTransform {
public:
    explicit RemovePunctuation() = default;

    void apply(CustomVector& data) override {
        char* read = data.getData();
        CustomVector result;

        while (*read) {
            if (!ispunct(*read)) {
                result.push_back(*read);
            }
            read++;
        }
        data.clear();

        for (size_t i = 0; i < result.getSize(); ++i) {
            data.push_back(result[i]);
        }
    }
};

class AddNewlineSentence : public TextTransform {
public:
    explicit AddNewlineSentence() = default;

    void apply(CustomVector& data) override {
        size_t length = data.getSize();
        CustomVector result;

        for (size_t i = 0; i < length; i++) {
            char currChar = data[i];
            result.push_back(currChar);
            if (currChar == '.' || currChar == '!' || currChar == '?') {
                if (i < length - 1) {
                    char nextChar = data[i + 1];
                    if (nextChar != '\n' && nextChar != '.' && nextChar != '\0') {
                        result.push_back('\n');
                    }
                }
            }
        }
        data.clear();

        for (size_t i = 0; i < result.getSize(); i++) {
            data.push_back(result[i]);
        }
    }
};

class AddNewlineWord : public TextTransform {
public:
    explicit AddNewlineWord() = default;

    void apply(CustomVector& data) override {
        CustomVector result;
        bool inWord = false;

        for (size_t i = 0; i < data.getSize(); ++i) {
            char c = data[i];
            result.push_back(c);
            if (isspace(c)) {
                if (inWord) {
                    result.push_back('\n');
                    inWord = false;
                }
            } else {
                inWord = true;
            }
        }
        data.clear();

        for (size_t i = 0; i < result.getSize(); ++i) {
            data.push_back(result[i]);
        }
    }
};

class AddNewlineMaxChars : public TextTransform {
    int maxCharsK;
public:
    explicit AddNewlineMaxChars(int maxCharsK) : maxCharsK(maxCharsK) {}

    void apply(CustomVector& data) override {
        size_t resIndex = 0;
        size_t currLineLen = 0;

        for (size_t i = 0; i < data.getSize(); i++) {
            char currentChar = data[i];
            if (currentChar != '\n') {
                currLineLen++;
            }
            if (currLineLen >= static_cast<size_t>(maxCharsK)) {
                for (size_t j = i; j > 0; j--) {
                    if (data[j] == ' ' || data[j] == '\n') {
                        data.insert(data.begin() + j, '\n');
                        currLineLen = i - j;
                        break;
                    }
                }
            }
            resIndex++;
        }
    }
};

class RemoveNewline : public TextTransform {
public:
    explicit RemoveNewline() = default;

    void apply(CustomVector& data) override {
        size_t resIndex = 0;

        for (size_t i = 0; i < data.getSize(); i++) {
            char currentChar = data[i];
            if (currentChar != '\n') {
                data[resIndex++] = currentChar;
            }
        }
        data.resize(resIndex);
    }
};

class LexSortLines : public TextTransform {
public:
    explicit LexSortLines() = default;

    void apply(CustomVector& data) override {
        char lines[maxLines][maxLineLen];
        int lineIndices[maxLines];
        int numLines = 0;

        char *token = strtok(data.getData(), "\n");
        while (token != nullptr) {
            strcpy(lines[numLines], token);
            lineIndices[numLines] = numLines;
            numLines++;
            token = strtok(nullptr, "\n");
        }

        sort(lineIndices, lineIndices + numLines, [&](int a, int b) {
            return strcmp(lines[a], lines[b]) < 0;
        });

        data.clear();

        for (int i = 0; i < numLines; i++) {
            const char *line = lines[lineIndices[i]];
            while (*line) {
                data.push_back(*line);
                line++;
            }
            if (i < numLines - 1) {
                data.push_back('\n');
            }
        }
    }
};

class RemoveDuplicateLines : public TextTransform {
public:
    explicit RemoveDuplicateLines() = default;

    void apply(CustomVector& data) override {
        char lines[maxLines][maxLineLen];
        int numLines = 0;

        char* token = strtok(data.getData(), "\n");
        while (token != nullptr && numLines < maxLines) {
            bool isDuplicate = false;
            for (int i = 0; i < numLines; i++) {
                if (strcmp(token, lines[i]) == 0) {
                    isDuplicate = true;
                    break;
                }
            }
            if (!isDuplicate) {
                strcpy(lines[numLines], token);
                numLines++;
            }
            token = strtok(nullptr, "\n");
        }
        data.clear();

        for (int i = 0; i < numLines; i++) {
            const char* line = lines[i];
            while (*line) {
                data.push_back(*line);
                line++;
            }
            if (i < numLines - 1) {
                data.push_back('\n');
            }
        }
    }
};

class CountLines : public TextTransform {
public:
    explicit CountLines() = default;

    void apply(CustomVector& data) override {
        int numLines = 0;
        char* read = data.getData();
        while (*read) {
            if (*read == '\n') {
                numLines++;
            }
            read++;
        }
        char numLinesStr[16];
        snprintf(numLinesStr, 16, "%d", numLines);

        data.clear();

        for (int i = 0; numLinesStr[i] != '\0'; i++) {
            data.push_back(numLinesStr[i]);
        }
    }
};

class CountSymbols : public TextTransform {
public:
    explicit CountSymbols() = default;

    void apply(CustomVector& data) override {
        int numSymbols = 0;
        char* read = data.getData();
        while (*read) {
            numSymbols++;
            read++;
        }
        char numSymbolsStr[16];
        snprintf(numSymbolsStr, 16, "%d", numSymbols);

        data.clear();

        for (int i = 0; numSymbolsStr[i] != '\0'; i++) {
            data.push_back(numSymbolsStr[i]);
        }
    }
};

class TextOutput {
public:
    explicit TextOutput() = default;

    virtual void writeData(const CustomVector& dataToWrite) = 0;
};

class TextConsoleOutput : public TextOutput {
public:
    explicit TextConsoleOutput() = default;

    void writeData(const CustomVector& dataToWrite) override {
        cout << dataToWrite;
    }
};

class TextFileOutput : public TextOutput {
    int maxSizeK;
    const char* fileName;
    int currFileSize;
    int fileIndex;
    ofstream outputFile;
    CustomVector data;
public:
    explicit TextFileOutput(int maxSizeK)
            : TextOutput(), maxSizeK(maxSizeK), fileName("../output"), currFileSize(0), fileIndex(0) {
        createNewFile();
    }

    TextFileOutput(const TextFileOutput& other) = delete;
    TextFileOutput& operator=(const TextFileOutput& other) = delete;

    void createNewFile() {
        if (outputFile.is_open()) {
            outputFile.close();
            fileIndex++;
        }
        char newFileName[100];
        snprintf(newFileName, sizeof(newFileName), "%s_%03d.txt", fileName, fileIndex);
        outputFile.open(newFileName);
    }

    void writeData(const CustomVector& dataToWrite) override {
        for (size_t i = 0; i < dataToWrite.getSize(); ++i) {
            data.push_back(dataToWrite[i]);
            if (currFileSize >= maxSizeK) {
                createNewFile();
                currFileSize = 0;
            }
            outputFile.write(&dataToWrite[i], 1);
            currFileSize++;
        }
    }
};

class TextProcessor {
    TextSource** sources;
    int numSources;
    TextTransform** transformations;
    int numTransformations;
    TextOutput** outputs;
    int numOutputs;

    CustomVector concatData;
public:
    TextProcessor(TextSource* sources[],
                  int numSources,
                  TextTransform* transformations[],
                  int numTransformations,
                  TextOutput* outputs[],
                  int numOutputs)
                    : sources(sources),
                      numSources(numSources),
                      transformations(transformations),
                      numTransformations(numTransformations),
                      outputs(outputs),
                      numOutputs(numOutputs) {}

    void concatenate(const char* data) {
        size_t dataLen = strlen(data);
        for (int i = 0; i < dataLen; ++i) {
            concatData.push_back(data[i]);
        }
    }

    void readFromSources() {
        for (int i = 0; i < numSources; ++i) {
            sources[i]->readData();
            const char* data = sources[i]->getData();
            if(data) {
                concatenate(data);
            }
        }
    }

    void applyTransformations() {
        for (int i = 0; i < numTransformations; ++i) {
                transformations[i]->apply(concatData);
        }
    }

    void outputSources() {
        for (int i = 0; i < numOutputs; ++i) {
            outputs[i]->writeData(concatData);
        }
    }

    void process() {
        readFromSources();
        applyTransformations();
        outputSources();
    }
};

int main() {
    TextFileSource source1("../data1.txt");
    TextFileSource source2("../data2.txt");
    TextConsoleSource source3;
    TextSource* sources[] = { &source1, &source2 ,&source3 };
    int numSources = (sizeof(sources) / sizeof(sources[0]));

    TextConsoleOutput consoleOutput;
    TextFileOutput fileOutput(200);
    TextOutput* outputs[] = {&consoleOutput, &fileOutput };
    int numOutputs = (sizeof(outputs) / sizeof(outputs[0]));

    RemoveString removeString("warlock");
    RemoveLines removeLines("Cataclysm");
    RemoveCharacter removeCharacter('t');
    ReplaceString replaceString("hope", "Horde");
    RemovePunctuation removePunctuation;
    AddNewlineSentence addNewlineSentence;
    AddNewlineWord addNewlineWord;
    AddNewlineMaxChars addNewlineMaxChars(20);
    RemoveNewline removeNewline;
    LexSortLines lexSortLines;
    RemoveDuplicateLines removeDuplicateLines;
    CountLines countLines;
    CountSymbols countSymbols;
    TextTransform* transformations[] = { &removeString,&removeNewline };
    TextTransform* transformations1[] = { &lexSortLines, &replaceString, &removePunctuation };
    TextTransform* transformations2[] = { &removeLines, &addNewlineSentence };
    TextTransform* transformations3[] = { &addNewlineWord, &removeString, &countSymbols };
    TextTransform* transformations4[] = { &lexSortLines, &removeDuplicateLines, &removeCharacter };
    TextTransform* transformations5[] = { &addNewlineMaxChars, &countLines };
    int numTransformations = (sizeof(transformations) / sizeof(transformations[0]));

    TextProcessor processor(sources, numSources, transformations, numTransformations, outputs, numOutputs);
//    processor.readFromSources();
//    processor.applyTransformations();
//    processor.outputSources();
    processor.process();

    return 0;
}