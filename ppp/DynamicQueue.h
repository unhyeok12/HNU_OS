#ifndef DYNAMICQUEUE_H
#define DYNAMICQUEUE_H

#include <iostream>
#include <mutex>

using namespace std;

extern mutex mtx;

struct Process {
    int id; // ���μ��� ID
    bool bg; // ��׶��� ���μ��� ����
    bool promoted; // ���θ�� ����
    int sleep_time;

    Process(int _id, bool _bg, int _sleep_time = 0) : id(_id), bg(_bg), promoted(false), sleep_time(_sleep_time) {}
};

struct ListNode {
    Process process; // ���μ���
    ListNode* next; // ���� ��� ������

    ListNode(Process _process) : process(_process), next(nullptr) {}
};

struct StackNode {
    ListNode* top; // 1�� ���μ��� ������
    StackNode* next; // ���� ���� ��� ������

    StackNode() : top(nullptr), next(nullptr) {}
};

class DynamicQueue {
private:
    StackNode* bottom; // ���� �� �Ʒ� ��� ������
    StackNode* top; // ���� �� �� ��� ������
    StackNode* P; // promote()�� ���� ��� ������
    int threshold;
    int totalProcesses; // ��ü ���μ��� ����
    int numStackNodes; // ���� ��� ��

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
