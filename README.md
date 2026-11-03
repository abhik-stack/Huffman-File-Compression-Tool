# Huffman File Compression Tool

A file compression and decompression application built using **C++ and Huffman Coding**, with a Flask-powered web dashboard.

## 1. Project Overview

The Huffman File Compression Tool uses the Huffman Coding algorithm to compress files by assigning variable-length binary codes to characters based on their frequency.

Characters that occur more frequently receive shorter binary codes, while less frequent characters receive longer codes.

The project combines Data Structures and Algorithms with a web-based interface to make file compression and decompression easy to use.

## 2. Features

- File compression using Huffman Coding.
- File decompression and restoration.
- Binary file storage using a custom HUF1 format.
- Huffman tree generation and visualization.
- Huffman binary code generation.
- Compression statistics, including original size, compressed size, and space saved.
- CSV export of Huffman codes.
- Automatic decompression verification after compression.
- Web dashboard built using HTML, CSS, and JavaScript.
- Flask backend for connecting the dashboard to the C++ engine.
- File upload validation and download functionality.

## 3. Technologies Used

| Technology | Purpose |
|---|---|
| C++ | Compression and decompression engine |
| Python | Backend integration |
| Flask | Web server and API |
| HTML | Dashboard structure |
| CSS | Dashboard styling |
| JavaScript | Frontend functionality |
| Graphviz | Huffman tree visualization |

## 4. Data Structures and Algorithms

The project demonstrates the following DSA concepts:

- **Greedy Algorithm:** Used to construct the Huffman Tree by repeatedly combining the two lowest-frequency nodes.
- **Binary Tree:** Used to represent the Huffman coding structure.
- **Min-Heap / Priority Queue:** Used to efficiently select the two nodes with the lowest frequencies.
- **Hash Map:** Used for character frequency counting and code storage.
- **Tree Traversal:** Used to generate binary Huffman codes.
- **Binary Encoding:** Used to store compressed data efficiently.

## 5. Project Structure

```text
FileCompressor/
│
├── main.cpp
├── huffman.cpp
├── huffman.h
├── decompress.cpp
├── compressor
│
├── server.py
│
├── frontend/
│   ├── index.html
│   ├── style.css
│   └── script.js
│
├── huffman_tree.dot
├── huffman_tree.png
├── huffman_codes.csv
│
├── uploads/
└── README.md
```

## 6. Installation and Execution

### Requirements

- C++17 compatible compiler
- Python 3
- Flask and Flask-CORS
- Graphviz (for tree visualization)

### Step 1: Compile the C++ program

```bash
clang++ -std=c++17 main.cpp huffman.cpp decompress.cpp -o compressor
```

### Step 2: Run the C++ application

```bash
./compressor
```

Select the required operation from the menu.

### Step 3: Start the Web Dashboard

Activate your Python environment and run:

```bash
source venv/bin/activate
python server.py
```

### Step 4: Open the Dashboard

Open the following address in your browser:

```text
http://127.0.0.1:5000
```

## 7. How It Works

### Compression

1. Read the input file.
2. Calculate the frequency of each character.
3. Insert character-frequency nodes into a min-heap.
4. Construct the Huffman Tree.
5. Generate binary codes through tree traversal.
6. Encode the original file using the generated codes.
7. Store the frequency information and compressed bitstream in the output file.
8. Calculate compression statistics.

### Decompression

1. Read the compressed file.
2. Validate the file format.
3. Reconstruct the Huffman Tree using the stored frequency information.
4. Decode the binary bitstream.
5. Restore the original file.
6. Verify the restored content where the original file is available.

## 8. Learning Outcomes

Through this project, we explored:

- Practical implementation of Huffman Coding.
- Greedy algorithm design.
- Binary tree construction and traversal.
- Priority queue implementation.
- Binary file handling in C++.
- Backend and frontend integration.
- File processing through APIs.
- Compression efficiency analysis.

## 9. Future Improvements

- Checksum-based corruption detection.
- Support for additional file formats.
- Improved compression performance.
- More detailed compression analytics.
- Native desktop application support.

## 10. Conclusion

The Huffman File Compression Tool demonstrates how fundamental Data Structures and Algorithms can solve a practical problem.

By combining C++, Python, and web technologies, the project provides an interactive environment for file compression, decompression, and visualization.

---

**Project:** Huffman File Compression Tool  
**Language:** C++17, Python, JavaScript  
**Category:** Advanced Data Structures and Algorithms