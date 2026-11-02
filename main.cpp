#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <limits>

#include "huffman.h"

using namespace std;
namespace fs = std::filesystem;


// ==========================================
// HELPER FUNCTION 1: WRITE VARIABLE-LENGTH INT
// ==========================================

void writeVarInt(ofstream& out, uint64_t value) {

    while (value >= 128) {

        unsigned char byte = (value & 127) | 128;

        out.put(static_cast<char>(byte));

        value >>= 7;
    }

    out.put(static_cast<char>(value));
}


// ==========================================
// HELPER FUNCTION 2: READ VARIABLE-LENGTH INT
// ==========================================

bool readVarInt(ifstream& in, uint64_t& value) {

    value = 0;
    int shift = 0;

    while (shift < 64) {

        char ch;

        if (!in.get(ch)) {
            return false;
        }

        unsigned char byte = static_cast<unsigned char>(ch);

        if (shift == 63 && byte > 1) {
            return false;
        }

        value |= static_cast<uint64_t>(byte & 127) << shift;

        if ((byte & 128) == 0) {
            return true;
        }

        shift += 7;
    }

    return false;
}


// ==========================================
// MAIN FUNCTION
// ==========================================

int main() {
        int choice;

    cout << "\n====================================\n";
    cout << "     HUFFMAN FILE COMPRESSION\n";
    cout << "====================================\n";
    cout << "1. Compress a file\n";
    cout << "2. Decompress a file\n";
    cout << "3. Exit\n";
    cout << "Enter your choice: ";

    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (choice == 3) {
        cout << "Exiting program...\n";
        return 0;
    }

    if (choice == 2) {
        string compressedName;
        string restoredName;

        cout << "Enter compressed file name: ";
        getline(cin, compressedName);

        cout << "Enter output file name: ";
        getline(cin, restoredName);

        decompressFile(compressedName, restoredName);
        return 0;
    }

    if (choice != 1) {
        cout << "Invalid choice!\n";
        return 1;
    }

    // ======================================
    // STEP 1: READ INPUT FILE
    // ======================================

    string inputFilename;

cout << "\nEnter the file name to compress: ";
getline(cin, inputFilename);
string compressedFilename = inputFilename + ".huff";
string restoredFilename = inputFilename + ".restored";

ifstream file(inputFilename, ios::binary);

if (!file.is_open()) {
    cout << "\nFile not found!";
    return 1;
}

    uint64_t frequency[256] = {0};

    char ch;

    while (file.get(ch)) {

        unsigned char index = static_cast<unsigned char>(ch);

        if (frequency[index] == UINT64_MAX) {
            cout << "\nFrequency overflow!";
            return 1;
        }

        frequency[index]++;
    }

    if (file.bad()) {

        cout << "\nError reading input file!";
        return 1;
    }

    file.close();

    cout << "\n========== CHARACTER FREQUENCIES ==========\n";

    uint16_t uniqueCount = 0;

    for (int i = 0; i < 256; i++) {

        if (frequency[i] > 0) {

            uniqueCount++;

            if (i == ' ') {
                cout << "[SPACE]";
            }
            else if (i == '\n') {
                cout << "[NEWLINE]";
            }
            else if (i == '\t') {
                cout << "[TAB]";
            }
            else {
                cout << static_cast<char>(i);
            }

            cout << " : " << frequency[i] << endl;
        }
    }

    // ======================================
    // STEP 2: BUILD HUFFMAN TREE
    // ======================================

    Node* root = buildHuffmanTree(frequency);

    if (root == nullptr) {

        cout << "\nInput file is empty.";

    }
    else {

        cout << "\nHuffman Tree Created Successfully!\n";

        cout << "Root Frequency: " << root->freq << endl;
    }


    // ======================================
    // STEP 3: GENERATE HUFFMAN CODES
    // ======================================

    string codes[256];

    generateCodes(root, "", codes);
    ofstream codeFile("huffman_codes.csv");

if (codeFile.is_open()) {

    codeFile << "Character,Frequency,Huffman Code\n";

    for (int i = 0; i < 256; i++) {

        if (frequency[i] > 0) {

            codeFile << i << ","
                     << frequency[i] << ","
                     << codes[i] << "\n";
        }
    }

    codeFile.close();

    cout << "\nHuffman code table exported successfully!";
    cout << "\nFile: huffman_codes.csv\n";
}
    // Export Huffman Tree
ofstream treeFile("huffman_tree.dot");

if (treeFile.is_open()) {

    exportTree(root, treeFile);

    treeFile.close();

    cout << "\nHuffman Tree exported successfully!";
    cout << "\nTree file: huffman_tree.dot\n";
}
else {
    cout << "\nCould not export Huffman Tree.\n";
}

    cout << "\n========== HUFFMAN CODES ==========\n";

    for (int i = 0; i < 256; i++) {

        if (frequency[i] > 0) {

            if (i == ' ') {
                cout << "[SPACE]";
            }
            else if (i == '\n') {
                cout << "[NEWLINE]";
            }
            else if (i == '\t') {
                cout << "[TAB]";
            }
            else {
                cout << static_cast<char>(i);
            }

            cout << " : " << codes[i] << endl;
        }
    }
    

    // ======================================
    // STEP 4: FILE COMPRESSION
    // ======================================

    ifstream input(inputFilename, ios::binary);

    ofstream output(compressedFilename,
                ios::binary | ios::trunc);

    if (!input || !output) {

        cout << "\nError opening compression files!";

        deleteTree(root);

        return 1;
    }


    // --------------------------------------
    // COMPACT HEADER FORMAT
    // --------------------------------------

    // Write file identifier

    output.write("HUF1", 4);


    // Write number of unique characters (2 bytes)

    output.put(static_cast<char>(uniqueCount & 0xFF));

    output.put(static_cast<char>((uniqueCount >> 8) & 0xFF));


    // Write only existing characters and frequencies

    for (int i = 0; i < 256; i++) {

        if (frequency[i] > 0) {

            output.put(static_cast<char>(i));

            writeVarInt(output, frequency[i]);
        }
    }


    // --------------------------------------
    // ENCODE CHARACTERS INTO BINARY
    // --------------------------------------

    unsigned char buffer = 0;

    int bitCount = 0;


    // A single-character file needs no encoded payload.
    // Its character and frequency are already in the header.

    bool singleCharacter =
        (root != nullptr &&
         root->left == nullptr &&
         root->right == nullptr);


    if (!singleCharacter) {

        while (input.get(ch)) {

            string code = codes[static_cast<unsigned char>(ch)];

            for (char bit : code) {

                buffer = (buffer << 1) | (bit - '0');

                bitCount++;

                if (bitCount == 8) {

                    output.put(static_cast<char>(buffer));

                    buffer = 0;

                    bitCount = 0;
                }
            }
        }
    }
    else {

        // Consume the input for the read-error check.
        while (input.get(ch)) {
        }
    }


    // Write remaining bits

    if (bitCount > 0) {

        buffer <<= (8 - bitCount);

        output.put(static_cast<char>(buffer));
    }


    // Check file operations

    bool inputError = input.bad();

    input.close();

    output.close();

    bool outputError = output.fail();

    if (inputError || outputError) {

        cout << "\nError during file compression!";

        deleteTree(root);

        return 1;
    }

    cout << "\nFile compressed successfully!";

    cout << "\nOutput file: " << compressedFilename << "\n";


    // ======================================
    // STEP 5: COMPRESSION STATISTICS
    // ======================================

    uintmax_t originalSize = fs::file_size(inputFilename);
uintmax_t compressedSize = fs::file_size(compressedFilename);

cout << "\n========== COMPRESSION REPORT ==========\n";

cout << "Original File Size   : "
     << originalSize << " bytes\n";

cout << "Compressed File Size : "
     << compressedSize << " bytes\n";

cout << fixed << setprecision(2);

if (originalSize > 0) {

    double saved = static_cast<double>(originalSize) -
                   static_cast<double>(compressedSize);

    double percentage = (saved / originalSize) * 100.0;

    double ratio = compressedSize > 0
                   ? static_cast<double>(originalSize) / compressedSize
                   : 0.0;

    cout << "Space Saved          : "
         << saved << " bytes\n";

    cout << "Compression Ratio    : "
         << ratio << ":1\n";

    cout << "Space Saving         : "
         << percentage << "%\n";
}
else {
    cout << "Empty file: no compression statistics available.\n";
}

cout << "========================================\n";


    // ======================================
    // STEP 6: FILE DECOMPRESSION
    // ======================================

    ifstream compressed(compressedFilename, ios::binary);

    ofstream restored(restoredFilename,
                  ios::binary | ios::trunc);

    if (!compressed || !restored) {

        cout << "\nError opening decompression files!";

        deleteTree(root);

        return 1;
    }


    uint64_t newFrequency[256] = {0};

    uint64_t totalCharacters = 0;

    bool valid = true;


    // --------------------------------------
    // READ AND VALIDATE FILE IDENTIFIER
    // --------------------------------------

    char magic[4];

    if (!compressed.read(magic, 4) ||
        string(magic, 4) != "HUF1") {

        cout << "\nInvalid compressed file format!";

        valid = false;
    }


    // --------------------------------------
    // READ UNIQUE CHARACTER COUNT
    // --------------------------------------

    uint16_t storedUniqueCount = 0;

    if (valid) {

        char lowByte, highByte;

        if (!compressed.get(lowByte) ||
            !compressed.get(highByte)) {

            cout << "\nInvalid compressed file header!";

            valid = false;
        }
        else {

            storedUniqueCount =
                static_cast<unsigned char>(lowByte) |
                (static_cast<uint16_t>(
                    static_cast<unsigned char>(highByte)
                ) << 8);

            if (storedUniqueCount > 256) {

                cout << "\nInvalid character count!";

                valid = false;
            }
        }
    }


    // --------------------------------------
    // READ CHARACTER FREQUENCIES
    // --------------------------------------

    if (valid) {

        for (int i = 0; i < storedUniqueCount; i++) {

            char character;

            uint64_t value = 0;

            if (!compressed.get(character) ||
                !readVarInt(compressed, value) ||
                value == 0) {

                cout << "\nInvalid frequency information!";

                valid = false;

                break;
            }

            unsigned char index =
                static_cast<unsigned char>(character);


            // Check duplicate characters and overflow

            if (newFrequency[index] != 0 ||
                UINT64_MAX - totalCharacters < value) {

                cout << "\nInvalid frequency header!";

                valid = false;

                break;
            }

            newFrequency[index] = value;

            totalCharacters += value;
        }
    }


    // --------------------------------------
    // REBUILD HUFFMAN TREE
    // --------------------------------------

    Node* decodeRoot = nullptr;

    if (valid) {

        decodeRoot = buildHuffmanTree(newFrequency);

        if (totalCharacters == 0 && storedUniqueCount != 0) {

            valid = false;
        }
        else if (totalCharacters > 0 && decodeRoot == nullptr) {

            valid = false;
        }
    }


    // --------------------------------------
    // HANDLE EMPTY FILE
    // --------------------------------------

    if (valid && totalCharacters == 0) {

        cout << "\nOriginal file was empty.";
    }


    // --------------------------------------
    // HANDLE SINGLE CHARACTER FILE
    // --------------------------------------

    else if (valid &&
             decodeRoot->left == nullptr &&
             decodeRoot->right == nullptr) {

        for (uint64_t i = 0; i < totalCharacters; i++) {

            restored.put(static_cast<char>(decodeRoot->ch));
        }
    }


    // --------------------------------------
    // DECODE MULTIPLE CHARACTERS
    // --------------------------------------

    else if (valid) {

        Node* current = decodeRoot;

        uint64_t decodedCharacters = 0;

        char byte;

        while (decodedCharacters < totalCharacters &&
               compressed.get(byte)) {

            unsigned char bits = static_cast<unsigned char>(byte);

            for (int i = 7; i >= 0; i--) {

                int bit = (bits >> i) & 1;

                if (bit == 0) {

                    current = current->left;
                }
                else {

                    current = current->right;
                }


                // Invalid tree traversal

                if (current == nullptr) {

                    valid = false;

                    break;
                }


                // Leaf node found

                if (current->left == nullptr &&
                    current->right == nullptr) {

                    restored.put(static_cast<char>(current->ch));

                    decodedCharacters++;

                    current = decodeRoot;

                    if (decodedCharacters == totalCharacters) {

                        break;
                    }
                }
            }

            if (!valid) {
                break;
            }
        }


        // Verify decoded character count

        if (decodedCharacters != totalCharacters) {

            valid = false;
        }
    }


    // --------------------------------------
    // CLOSE FILES AND CHECK ERRORS
    // --------------------------------------

    restored.close();

    compressed.close();

    bool restoredError = restored.fail();

    deleteTree(decodeRoot);

    deleteTree(root);


    if (!valid || restoredError) {

        cout << "\nDecompression failed!";

        return 1;
    }

    cout << "\nFile decompressed successfully!";

    cout << "\nOutput file: " << restoredFilename << "\n";


    // ======================================
    // STEP 7: VERIFY ORIGINAL AND RESTORED
    // ======================================

    ifstream original(inputFilename, ios::binary);

    ifstream recovered(restoredFilename, ios::binary);

    if (!original || !recovered) {

        cout << "\nCould not open files for verification!";

        return 1;
    }


    bool identical = true;

    char originalByte;

    char recoveredByte;


    while (true) {

        bool originalRead = static_cast<bool>(
            original.get(originalByte)
        );

        bool recoveredRead = static_cast<bool>(
            recovered.get(recoveredByte)
        );


        if (originalRead != recoveredRead) {

            identical = false;

            break;
        }


        if (!originalRead) {

            break;
        }


        if (originalByte != recoveredByte) {

            identical = false;

            break;
        }
    }


    if (original.bad() || recovered.bad()) {

        identical = false;
    }


    original.close();

    recovered.close();


    if (identical) {

        cout << "\nVerification: SUCCESS";

        cout << "\nOriginal and restored files are identical.\n";
    }
    else {

        cout << "\nVerification: FAILED\n";
    }


    // ======================================
    // PROGRAM FINISHED
    // ======================================

    return identical ? 0 : 1;
}