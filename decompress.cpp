
#include "huffman.h"
#include <fstream>
#include <cstdint>
#include <limits>

using namespace std;

// Read variable-length integer
bool readVarIntForDecompression(ifstream& in, uint64_t& value) {
    value = 0;
    int shift = 0;

    while (shift < 64) {
        char ch;

        if (!in.get(ch))
            return false;

        unsigned char byte = static_cast<unsigned char>(ch);

        if (shift == 63 && byte > 1)
            return false;

        value |= static_cast<uint64_t>(byte & 127) << shift;

        if ((byte & 128) == 0)
            return true;

        shift += 7;
    }

    return false;
}

// Independent decompression function
bool decompressFile(const string& inputFilename,
                    const string& outputFilename) {

    ifstream compressed(inputFilename, ios::binary);

    if (!compressed.is_open()) {
        cout << "Cannot open compressed file!\n";
        return false;
    }

    // Check file identifier
    char magic[4];

    if (!compressed.read(magic, 4) ||
        string(magic, 4) != "HUF1") {
        cout << "Invalid compressed file!\n";
        return false;
    }

    // Read unique character count
    char low, high;

    if (!compressed.get(low) || !compressed.get(high)) {
        cout << "Invalid file header!\n";
        return false;
    }

    uint16_t uniqueCount =
        static_cast<unsigned char>(low) |
        (static_cast<uint16_t>(
            static_cast<unsigned char>(high)) << 8);

    if (uniqueCount > 256) {
        cout << "Invalid character count!\n";
        return false;
    }

    // Read character frequencies
    uint64_t frequency[256] = {0};
    uint64_t totalCharacters = 0;

    for (int i = 0; i < uniqueCount; i++) {
        char ch;
        uint64_t value;

        if (!compressed.get(ch) ||
            !readVarIntForDecompression(compressed, value) ||
            value == 0) {
            cout << "Invalid frequency information!\n";
            return false;
        }

        unsigned char index =
            static_cast<unsigned char>(ch);

        if (frequency[index] != 0 ||
            numeric_limits<uint64_t>::max()
                - totalCharacters < value) {
            cout << "Invalid frequency header!\n";
            return false;
        }

        frequency[index] = value;
        totalCharacters += value;
    }

    // Build Huffman tree
    Node* root = buildHuffmanTree(frequency);

    if ((totalCharacters == 0 && uniqueCount != 0) ||
        (totalCharacters > 0 && root == nullptr)) {
        deleteTree(root);
        cout << "Invalid Huffman tree!\n";
        return false;
    }

    ofstream restored(outputFilename,
                      ios::binary | ios::trunc);

    if (!restored.is_open()) {
        deleteTree(root);
        cout << "Cannot create output file!\n";
        return false;
    }

    // Handle empty file
    if (totalCharacters == 0) {
        cout << "Empty file decompressed.\n";
    }

    // Handle single-character file
    else if (root->left == nullptr &&
             root->right == nullptr) {

        for (uint64_t i = 0; i < totalCharacters; i++)
            restored.put(static_cast<char>(root->ch));
    }

    // Decode multiple characters
    else {
        Node* current = root;
        uint64_t decoded = 0;
        char byte;

        while (decoded < totalCharacters &&
               compressed.get(byte)) {

            unsigned char bits =
                static_cast<unsigned char>(byte);

            for (int i = 7; i >= 0; i--) {

                int bit = (bits >> i) & 1;

                current = bit == 0
                    ? current->left
                    : current->right;

                if (current == nullptr) {
                    cout << "Invalid compressed data!\n";
                    restored.close();
                    deleteTree(root);
                    return false;
                }

                if (current->left == nullptr &&
                    current->right == nullptr) {

                    restored.put(static_cast<char>(current->ch));

                    decoded++;
                    current = root;

                    if (decoded == totalCharacters)
                        break;
                }
            }
        }

        if (decoded != totalCharacters) {
            cout << "Compressed file is incomplete!\n";
            restored.close();
            deleteTree(root);
            return false;
        }
    }

    restored.close();
    compressed.close();

    bool success = !restored.fail();

    deleteTree(root);

    if (!success) {
        cout << "Error writing restored file!\n";
        return false;
    }

    cout << "Decompression successful!\n";
    cout << "Restored file: " << outputFilename << "\n";

    return true;
}
