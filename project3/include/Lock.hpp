#pragma once
#ifndef __LOCK_HPP__
#define __LOCK_HPP__

#include <unordered_map>
#include <deque>
#include <pthread.h>
#include "Defs.hpp"

enum LockMode
{
    READ,
    WRITE
};

class LockTable
{
private:
    class LockNode
    {
    public:
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
    };

    class LockList
    {
    public:
        LockNode *head;
        LockNode *tail;
        LockList() : head(nullptr),
                     tail(nullptr)
        {
            // Do Nothing
        }
    }

    pthread_mutex_t globalMtx;
    std::unordered_map<TransactionId, std::deque<LockNode *>> lockPerTxn;
    std::unordered_map<RecordId, LockList> lockPerRecord;

    bool checkDeadlock(TransactionId txnId, RecordId recordId);
    LockNode *addReaderLock(TransactionId txnId, RecordId recordId);
    LockNode *addWriterLock(TransactionId txnId, RecordId recordId);

public:
    bool lock(TransactionId txnId, RecordId recordId, LockMode lockMode);
    void unlock(TransactionId txnId);
};

#endif