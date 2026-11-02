#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <string>
#include <iostream>
#include <queue>
#include <vector>
#include <cstdint>
#include <fstream>

using namespace std;

struct Node {
    unsigned char ch;
    uint64_t freq;

    Node* left;
    Node* right;

    Node(unsigned char c, uint64_t f) {
        ch = c;
        freq = f;
        left = nullptr;
        right = nullptr;
    }
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

Node* buildHuffmanTree(uint64_t frequency[]);

void deleteTree(Node* root);

void generateCodes(Node* root, string code, string codes[]);
void exportTree(Node* root, ofstream& output);
bool decompressFile(const string& inputFilename,
                    const string& outputFilename);
#endif