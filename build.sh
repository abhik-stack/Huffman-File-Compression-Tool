#!/usr/bin/env bash
set -o errexit

g++ -std=c++17 main.cpp huffman.cpp decompress.cpp -o compressor