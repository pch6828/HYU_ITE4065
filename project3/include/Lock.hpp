#pragma once
#ifndef __LOCK_HPP__
#define __LOCK_HPP__

#include <vector>
#include <deque>
#include <pthread.h>
#include "Defs.hpp"

enum LockMode
{
    READ,
    WRITE
};
class LockNode
{
private:
    bool locked;
    LockMode lockMode;
    TransactionId txnId;
    RecordId recordId;
    LockNode *nextInTxn;
    LockNode *prev;
    LockNode *next;

    LockNode(LockMode lockMode, TransactionId txnId, RecordId recordId) : locked(false),
                                                                          lockMode(lockMode),
                                                                          txnId(txnId),
                                                                          recordId(recordId),
                                                                          nextInTxn(nullptr),
                                                                          prev(nullptr),
                                                                          next(nullptr)
    {
        // Do Nothing
    }

    friend class LockList;
    friend class LockTable;
};

class LockList
{
private:
    LockNode *head;
    LockNode *tail;
    LockList() : head(nullptr),
                 tail(nullptr)
    {
        // Do Nothing
    }

    friend class LockTable;
};

class LockTable
{
private:
    pthread_mutex_t globalMtx;
    std::vector<std::deque<LockNode *>> lockPerTxn;
    std::vector<LockList> lockPerRecord;

    bool checkDeadlock(TransactionId txnId, RecordId recordId);
    LockNode *addReaderLock(TransactionId txnId, RecordId recordId);
    LockNode *addWriterLock(TransactionId txnId, RecordId recordId);

public:
    LockTable(int64_t numRecord, int64_t numTxn) : lockPerTxn(numTxn + 1),
                                                   lockPerRecord(numRecord + 1, LockList())
    {
        globalMtx = PTHREAD_MUTEX_INITIALIZER;
    }

    bool lock(TransactionId txnId, RecordId recordId, LockMode lockMode);
    void unlock(TransactionId txnId);
};

#endif