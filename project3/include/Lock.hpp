#pragma once
#ifndef __LOCK_HPP__
#define __LOCK_HPP__

#include <vector>
#include <deque>
#include <pthread.h>
#include "Defs.hpp"
//---------------------------------------------------------------------------
/// Locking Mode for Single Lock
enum LockMode
{
    READ,
    WRITE
};
//---------------------------------------------------------------------------
class LockNode
{
    /// Definition For Single Lock Object
private:
    bool locked;         // Flag Variable for Spin-Waiting
    LockMode lockMode;   // Locking Mode
    TransactionId txnId; // Transaction Id Acquiring This Lock
    RecordId recordId;   // Record Id Locked by This Lock
    LockNode *nextInTxn; // Next Acquiring/Acquired Lock In Single Transaction
    LockNode *prev;      // Previous Lock Object
                         // It will wait until this lock is released.
    LockNode *next;      // Next Lock Object
                         // It will break this lock's wait loop when it is released.

    /// Constructor for New Lock Object
    /// At Now, All pointer of LockNode is initialized as `nullptr`
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
//---------------------------------------------------------------------------
class LockList
{
    /// Definition For Lock List of Single Record
private:
    /// Since it is doubley-linked list, it has head and tail
    LockNode *head;
    LockNode *tail;
    /// Default Constructor
    LockList() : head(nullptr),
                 tail(nullptr)
    {
        // Do Nothing
    }

    friend class LockTable;
};
//---------------------------------------------------------------------------
class LockTable
{
    /// Definition For LockTable
    /// This is general interface for locking mechanism of total DB
private:
    pthread_mutex_t globalMtx;                      // Global Mutex for Protecting LockTable Structure.
    std::vector<std::deque<LockNode *>> lockPerTxn; // Table to Track Each Transaction's Lock Object
    std::vector<LockList> lockPerRecord;            // Each Record's Lock List

    /// Checking Deadlock by Traversing Wait-For Graph
    bool checkDeadlock(TransactionId txnId, RecordId recordId);
    /// Add Reader Lock Atomically
    /// If there are deadlock, return nullptr
    LockNode *addReaderLock(TransactionId txnId, RecordId recordId);
    /// Add Writer Lock Atomically
    /// If there are deadlock, return nullptr
    LockNode *addWriterLock(TransactionId txnId, RecordId recordId);

public:
    /// Constructor for Lock Table
    /// Just allocate slots for internal tables.
    LockTable(int64_t numRecord, int64_t numTxn) : lockPerTxn(numTxn + 1),
                                                   lockPerRecord(numRecord + 1, LockList())
    {
        // initialize global mutex
        globalMtx = PTHREAD_MUTEX_INITIALIZER;
    }

    /// Try Acquire Lock Atomically, And Wait for It
    /// return false if there is an deadlock
    bool lock(TransactionId txnId, RecordId recordId, LockMode lockMode);
    /// Unlock Locks Atomically
    /// Release all lock objects acquired by given transaction
    void unlock(TransactionId txnId);
};
//---------------------------------------------------------------------------
#endif