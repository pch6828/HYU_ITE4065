#include "Lock.hpp"
#include <queue>
#include <iostream>

using namespace std;
//---------------------------------------------------------------------------
/*
 * PAGE 1 : Function Definition for LockTable
 */
bool LockTable::checkDeadlock(TransactionId txnId, RecordId recordId)
// Checking Deadlock by Traversing Wait-For Graph
{
    // lock list of given record Id
    LockList &lockList = lockPerRecord[recordId];

    // queue for BFS algorithm
    queue<LockNode *> waitQueue;
    // add last lock of given record
    waitQueue.push(lockList.tail);

    // start BFS
    while (!waitQueue.empty())
    {
        // get lock object in Wait-For Graph
        LockNode *current = waitQueue.front();
        waitQueue.pop();

        // since current lock object is valid,
        if (current)
        {
            if (current->txnId == txnId)
            {
                // if current lock was acquired by given transaction,
                // there is an DEADLOCK
                return true;
            }
            // if deadlock is not detected yet,
            // add next lock objects into Wait-For Graph
            waitQueue.push(current->prev);
            waitQueue.push(current->nextInTxn);
        }
    }

    // BFS complete, there is no deadlock
    return false;
}

LockNode *LockTable::addReaderLock(TransactionId txnId, RecordId recordId)
// Add Reader Lock Atomically
// If there are deadlock, return nullptr
{
    // acquire global mutex to protect LockTable structure
    pthread_mutex_lock(&globalMtx);

    if (checkDeadlock(txnId, recordId))
    {
        // if adding reader lock can cause deadlock,
        // no lock object is constructed, return `nullptr`
        // also, release global mutex
        pthread_mutex_unlock(&globalMtx);
        return nullptr;
    }

    // adding reader lock is safe,
    // so continue remaining step.

    // make new lock object
    LockNode *newLock = new LockNode(READ, txnId, recordId);

    // build Wait-For Graph
    // step 1 : connect new lock with last acquired lock by given transaction
    deque<LockNode *> &lockInTxn = lockPerTxn[txnId];
    if (!lockInTxn.empty())
    {
        lockInTxn.back()->nextInTxn = newLock;
    }

    // step 2 : add new lock to back of per-record lock list
    LockList &lockList = lockPerRecord[recordId];

    if (lockList.tail == nullptr)
    {
        // if lock list is empty,
        // adjust it
        lockList.head = lockList.tail = newLock;
    }
    else
    {
        // if lock list is not empty,
        // connect new lock to tail
        newLock->prev = lockList.tail;
        lockList.tail->next = newLock;
        // set lock list's tail with new lock
        lockList.tail = newLock;
    }

    // set lock state by traversing lock list backward
    LockNode *iter = lockList.tail;
    while (iter)
    {
        // if there are write lock in lock list,
        // set locked state `true`
        // so it should wait to acquire lock
        if (iter->lockMode == WRITE)
        {
            newLock->locked = true;
            break;
        }
        iter = iter->prev;
    }

    // add new lock into per-transaction table
    lockInTxn.push_back(newLock);
    // release global mutex
    pthread_mutex_unlock(&globalMtx);
    // return new lock object
    return newLock;
}

LockNode *LockTable::addWriterLock(TransactionId txnId, RecordId recordId)
// Add Writer Lock Atomically
// If there are deadlock, return nullptr
{
    // acquire global mutex to protect LockTable structure
    pthread_mutex_lock(&globalMtx);

    if (checkDeadlock(txnId, recordId))
    {
        // if adding writer lock can cause deadlock,
        // no lock object is constructed, return `nullptr`
        // also, release global mutex
        pthread_mutex_unlock(&globalMtx);
        return nullptr;
    }

    // adding writer lock is safe,
    // so continue remaining step.

    // make new lock object
    LockNode *newLock = new LockNode(WRITE, txnId, recordId);

    // build Wait-For Graph
    // step 1 : connect new lock with last acquired lock by given transaction
    deque<LockNode *> &lockInTxn = lockPerTxn[txnId];
    if (!lockInTxn.empty())
    {
        lockInTxn.back()->nextInTxn = newLock;
    }

    // step 2 : add new lock to back of per-record lock list
    LockList &lockList = lockPerRecord[recordId];

    if (lockList.tail == nullptr)
    {
        // if lock list is empty,
        // adjust it
        lockList.head = lockList.tail = newLock;
    }
    else
    {
        // if lock list is not empty,
        // connect new lock to tail
        newLock->prev = lockList.tail;
        lockList.tail->next = newLock;
        // set lock list's tail with new lock
        lockList.tail = newLock;
        // since there are one or more lock in lock list,
        // set locked state `true`
        // so it should wait to acquire lock
        newLock->locked = true;
    }

    // add new lock into per-transaction table
    lockInTxn.push_back(newLock);
    // release global mutex
    pthread_mutex_unlock(&globalMtx);
    // return new lock object
    return newLock;
}

bool LockTable::lock(TransactionId txnId, RecordId recordId, LockMode lockMode)
// Try Acquire Lock Atomically, And Wait for It
// return false if there is an deadlock
{
    LockNode *myLock = nullptr;
    // try adding lock object atomically
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
        // if there is DEADLOCK,
        // acquiring lock is failed
        return false;
    }

    while (myLock->locked)
    {
        // spin wait until my lock's state is not locked.
    }

    // acquiring lock is successed.
    return true;
}

void LockTable::unlock(TransactionId txnId)
// Unlock Locks Atomically
// Release all lock objects acquired by given transaction
{
    // acquire global mutex to protect LockTable structure
    pthread_mutex_lock(&globalMtx);

    deque<LockNode *> &lockInTxn = lockPerTxn[txnId];
    // for all lock objects acquired by given transaction,
    for (LockNode *myLock : lockInTxn)
    {
        // remove lock from lock list which it is belong to
        LockList &lockList = lockPerRecord[myLock->recordId];
        LockNode *prevLock = myLock->prev;
        LockNode *nextLock = myLock->next;

        // if lock is the head of lock list,
        // then it should wake up next threads
        if (lockList.head == myLock)
        {
            // if there is next lock, wake up sequence follows
            if (nextLock)
            {
                // set lock list's head with next lock, and cut the connection with my lock
                lockList.head = nextLock;
                nextLock->prev = nullptr;

                if (nextLock->lockMode == WRITE)
                {
                    // if next lock is writer lock,
                    // just wake up single thread
                    nextLock->locked = false;
                }
                else
                {
                    // if next lock is reader lock,
                    // wake up all consecutive threads trying read operation.
                    LockNode *iter = nextLock;
                    while (iter && iter->lockMode == READ)
                    {
                        iter->locked = false;
                        iter = iter->next;
                    }
                }
            }
            // if there is no next lock,
            // lock list should be empty
            else
            {
                // so set its head and tail with `nullptr`
                lockList.head = lockList.tail = nullptr;
            }
        }
        // if lock is not the head of lock list,
        // just remove lock from lock list
        else
        {
            // remove connection with my lock
            prevLock->next = nextLock;
            if (nextLock)
            {
                nextLock->prev = prevLock;
            }
            else
            {
                // if my lock was tail of lock list,
                // set locklist's tail with previous lock object
                lockList.tail = prevLock;
            }
        }
        // at this point, my lock does not have connection with lock list,
        // so we can deallocate it safely
        delete myLock;
    }
    // clear the per-transaction table
    lockInTxn.clear();
    // release global mutex
    pthread_mutex_unlock(&globalMtx);
}
//---------------------------------------------------------------------------
