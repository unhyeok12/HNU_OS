#include "DynamicQueue.h"
mutex mtx;

DynamicQueue::DynamicQueue(int _threshold) : bottom(nullptr), top(nullptr), P(nullptr), threshold(_threshold), totalProcesses(0), numStackNodes(0) {}

DynamicQueue::~DynamicQueue() {
    while (bottom) {
        StackNode* temp = bottom;
        bottom = bottom->next;
        while (temp->top) {
            ListNode* tempNode = temp->top;
            temp->top = temp->top->next;
            delete tempNode;
        }
        delete temp;
    }
}

void DynamicQueue::insertToList(ListNode*& head, const Process& process) {
    ListNode* newNode = new ListNode(process);
    if (!head) {
        head = newNode;
    }
    else {
        ListNode* curr = head;
        while (curr->next) {
            curr = curr->next;
        }
        curr->next = newNode;
    }
}

void DynamicQueue::removeFromList(ListNode*& head) {
    if (!head) return;
    ListNode* temp = head;
    head = head->next;
    delete temp;
}

void DynamicQueue::enqueue(const Process& process) {
    lock_guard<mutex> lock(mtx);
    if (!bottom) {
        bottom = new StackNode();
        top = bottom;
        numStackNodes++;
    }
    if (process.bg) {
        insertToList(bottom->top, process);
    }
    else {
        insertToList(top->top, process);
    }
    totalProcesses++;
    if (P) promote();
    split_n_merge();
}

void DynamicQueue::dequeue() {
    lock_guard<mutex> lock(mtx);
    if (!bottom || !bottom->top) return;
    removeFromList(bottom->top);
    if (!bottom->top) {
        StackNode* temp = bottom;
        bottom = bottom->next;
        delete temp;
        numStackNodes--;
    }
    totalProcesses--;
}

void DynamicQueue::promote() {
    lock_guard<mutex> lock(mtx);
    if (!P) {
        P = bottom;
    }
    if (P->top && !P->top->process.promoted) {
        P->top->process.promoted = true;
        insertToList(top->top, P->top->process);
        removeFromList(P->top);
        if (!P->top) {
            StackNode* temp = P;
            P = P->next ? P->next : bottom;
            delete temp;
            numStackNodes--;
        }
    }
    P = P->next ? P->next : bottom;
}

void DynamicQueue::split_n_merge() {
    StackNode* curr = bottom;
    while (curr) {
        int count = 0;
        ListNode* temp = curr->top;
        while (temp) {
            ++count;
            temp = temp->next;
        }
        if (count > threshold) {
            int mid = count / 2;
            ListNode* midNode = curr->top;
            for (int i = 1; i < mid; ++i) {
                midNode = midNode->next;
            }
            StackNode* newStack = new StackNode();
            newStack->top = midNode->next;
            midNode->next = nullptr;
            newStack->next = curr->next;
            curr->next = newStack;
            numStackNodes++;
        }
        else {
            curr = curr->next;
        }
    }
}


void DynamicQueue::printQueueState() {
    lock_guard<mutex> lock(mtx);
    StackNode* curr = bottom;
    while (curr) {
        ListNode* temp = curr->top;
        while (temp) {
            cout << temp->process.id << (temp->process.bg ? "B" : "F");
            if (temp->process.promoted) {
                cout << "*";
            }
            cout << " ";
            temp = temp->next;
        }
        curr = curr->next;
    }
    cout << "]" << endl;
}
