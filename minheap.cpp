#include <iostream>
#include <queue>
#include <vector>
using namespace std;

int main() {

    priority_queue<int, vector<int>, greater<int>> minHeap;

    minHeap.push(6);
    minHeap.push(2);
    minHeap.push(3);
    minHeap.push(3);
    minHeap.push(3);

    cout << "Min Heap Elements:\n";

    while (!minHeap.empty()) {
        cout << minHeap.top() << " ";
        minHeap.pop();
    }

    return 0;
}