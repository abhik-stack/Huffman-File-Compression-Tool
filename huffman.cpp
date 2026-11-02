#include "huffman.h"

Node* buildHuffmanTree(uint64_t frequency[]) {

    priority_queue<Node*, vector<Node*>, Compare> minHeap;

    // Create leaf nodes
    for (int i = 0; i < 256; i++) {

        if (frequency[i] > 0) {

            Node* newNode = new Node(
                (unsigned char)i,
                frequency[i]
            );

            minHeap.push(newNode);
        }
    }

    // Empty file
    if (minHeap.empty()) {
        return nullptr;
    }

    // Build Huffman Tree
    while (minHeap.size() > 1) {

        Node* left = minHeap.top();
        minHeap.pop();

        Node* right = minHeap.top();
        minHeap.pop();

        Node* parent = new Node(
            '\0',
            left->freq + right->freq
        );

        parent->left = left;
        parent->right = right;

        minHeap.push(parent);
    }

    return minHeap.top();
}

// Free allocated memory
void deleteTree(Node* root) {

    if (root == nullptr) {
        return;
    }

    deleteTree(root->left);
    deleteTree(root->right);

    delete root;
}
void generateCodes(Node* root, string code, string codes[]) {

    if (root == nullptr) {
        return;
    }

    // Leaf node found
    if (root->left == nullptr && root->right == nullptr) {

    codes[root->ch] = code.empty() ? "0" : code;
    return;
}

    // Left = 0
    generateCodes(root->left, code + "0", codes);

    // Right = 1
    generateCodes(root->right, code + "1", codes);
}
// Helper function to export tree nodes
int writeTreeNodes(Node* root, ofstream& output, int& nextId) {

    if (root == nullptr)
        return -1;

    int currentId = nextId++;

    // Check whether this is a leaf node
    if (root->left == nullptr && root->right == nullptr) {

        output << "  node" << currentId
               << " [label=\"ASCII: "
               << static_cast<int>(root->ch)
               << "\\nFrequency: " << root->freq
               << "\"];\n";
    }
    else {

        output << "  node" << currentId
               << " [label=\"Internal\\nFrequency: "
               << root->freq << "\"];\n";
    }

    int leftId = writeTreeNodes(root->left, output, nextId);

    if (leftId != -1) {
        output << "  node" << currentId
               << " -> node" << leftId
               << " [label=\"0\"];\n";
    }

    int rightId = writeTreeNodes(root->right, output, nextId);

    if (rightId != -1) {
        output << "  node" << currentId
               << " -> node" << rightId
               << " [label=\"1\"];\n";
    }

    return currentId;
}


// Export Huffman tree
void exportTree(Node* root, ofstream& output) {

    output << "digraph HuffmanTree {\n";
    output << "  node [shape=circle];\n";

    int nextId = 0;

    writeTreeNodes(root, output, nextId);

    output << "}\n";
}