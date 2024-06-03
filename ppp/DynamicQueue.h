#ifndef DYNAMICQUEUE_H
#define DYNAMICQUEUE_H

#include <iostream>
#include <mutex>

using namespace std;

extern mutex mtx;

struct Process {
    int id; // 프로세스 ID
    bool bg; // 백그라운드 프로세스 여부
    bool promoted; // 프로모션 여부
    int sleep_time;

    Process(int _id, bool _bg, int _sleep_time = 0) : id(_id), bg(_bg), promoted(false), sleep_time(_sleep_time) {}
};

struct ListNode {
    Process process; // 프로세스
    ListNode* next; // 다음 노드 포인터

    ListNode(Process _process) : process(_process), next(nullptr) {}
};

struct StackNode {
    ListNode* top; // 1번 프로세스 포인터
    StackNode* next; // 다음 스택 노드 포인터

    StackNode() : top(nullptr), next(nullptr) {}
};

class DynamicQueue {
private:
    StackNode* bottom; // 스택 맨 아래 노드 포인터
    StackNode* top; // 스택 맨 위 노드 포인터
    StackNode* P; // promote()할 스택 노드 포인터
    int threshold;
    int totalProcesses; // 전체 프로세스 개수
    int numStackNodes; // 스택 노드 수

    void insertToList(ListNode*& head, const Process& process);
    void removeFromList(ListNode*& head);

public:
    DynamicQueue(int _threshold);
    ~DynamicQueue();

    void enqueue(const Process& process);
    void dequeue();
    void promote();
    void split_n_merge();
    void printQueueState();
};

#endif
