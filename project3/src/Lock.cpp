#include "Lock.hpp"
#include <queue>

using namespace std;

bool LockTable::checkDeadlock(TransactionId txnId, RecordId recordId)
{
    LockList &lockList = lockPerRecord[recordId];

    queue<LockNode *> waitQueue;
    waitQueue.push(lockList.tail);

    while (!waitQueue.empty())
    {
        LockNode *current = waitQueue.front();
        waitQueue.pop();

        if (current)
        {
            if (current->txnId == txnId)
            {
                return true;
            }
            waitQueue.push(current->prev);
            waitQueue.push(current->nextInTxn);
        }
    }

    return false;
}

LockNode *LockTable::addReaderLock(TransactionId txnId, RecordId recordId)
{
    pthread_mutex_lock(&globalMtx);
    if (checkDeadlock(txnId, recordId))
    {
        pthread_mutex_unlock(&globalMtx);
        return nullptr;
    }

    LockNode *newLock = new LockNode(READ, txnId, recordId);
    deque<LockNode *> &prevLockInTxn = lockPerTxn[txnId];
    if (!prevLockInTxn.empty())
    {
        prevLockInTxn.back()->nextInTxn = newLock;
    }
    LockList &lockList = lockPerRecord[recordId];

    if (lockList.tail == nullptr)
    {
        lockList.head = lockList.tail = newLock;
    }
    else
    {
        newLock->prev = lockList.tail;
        lockList.tail->next = newLock;
        lockList.tail = newLock;
    }

    LockNode *iter = lockList.tail;
    while (iter)
    {
        if (iter->lockMode == WRITE)
        {
            newLock->locked = true;
            break;
        }
        iter = iter->prev;
    }

    pthread_mutex_unlock(&globalMtx);
    return newLock;
}

LockNode *LockTable::addWriterLock(TransactionId txnId, RecordId recordId)
{
    pthread_mutex_lock(&globalMtx);
    if (checkDeadlock(txnId, recordId))
    {
        pthread_mutex_unlock(&globalMtx);
        return nullptr;
    }

    LockNode *newLock = new LockNode(WRITE, txnId, recordId);
    deque<LockNode *> &prevLockInTxn = lockPerTxn[txnId];
    if (!prevLockInTxn.empty())
    {
        prevLockInTxn.back()->nextInTxn = newLock;
    }
    LockList &lockList = lockPerRecord[recordId];

    if (lockList.tail == nullptr)
    {
        lockList.head = lockList.tail = newLock;
    }
    else
    {
        newLock->prev = lockList.tail;
        lockList.tail->next = newLock;
        lockList.tail = newLock;
        newLock->locked = true;
    }

    pthread_mutex_unlock(&globalMtx);
    return newLock;
}

bool LockTable::lock(TransactionId txnId, RecordId recordId, LockMode lockMode)
{
    LockNode *myLock = nullptr;
    if (lockMode == READ)
    {
        myLock = addReaderLock(txnId, recordId);
    }
    else
    {
        myLock = addWriterLock(txnId, recordId);
    }

    if (!myLock)
    {
        return false;
    }

    while (myLock->locked)
    {
        // Spin Wait
    }

    return true;
}

void unlock(TransactionId txnId)
{
    pthread_mutex_lock(&globalMtx);

    deque<LockNode *> &lockInTxn = lockPerTxn[txnId];

    for (LockNode *myLock : lockInTxn)
    {
        LockList &lockList = lockPerRecord[myLock->recordId];
        LockNode *prevLock = myLock->prev;
        LockNode *nextLock = myLock->next;
        if (lockList.head == myLock)
        {
            if (nextLock)
            {
                nextLock->locked = false;
                lockList.head = nextLock;
            }
            else
            {
                lockList.head = lockList.tail = nullptr;
            }
        }
        else
        {
            prevLock->next = nextLock;
            if (nextLock)
            {
                nextLock->prev = prevLock;
            }
        }
        delete myLock;
    }
    pthread_mutex_unlock(&globalMtx);
}