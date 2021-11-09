#pragma once
#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <fstream>
#include <string>
#include <deque>
#include <unordered_map>
#include <pthread.h>
#include "Defs.hpp"
//---------------------------------------------------------------------------
class LogForUndo
{
    /// Definition for Special Log for UNDO Operation
    /// instance of this should be read-only
public:
    /// Fields for Log Entry
    const std::deque<RecordId> *recordIds; // Record Ids Accessed by This Transaction
    const std::deque<Value> *beforeValues; // Values That Should Be Recovered
    /// Default Constructor
    LogForUndo(const std::deque<RecordId> *recordIds,
               const std::deque<Value> *beforeValues) : recordIds(recordIds),
                                                        beforeValues(beforeValues)
    {
        // Do Nothing
    }
};
//---------------------------------------------------------------------------
class Logger
{
    /// Definition for Log Manager
    /// This is general interface for logging mechanism of total DB
private:
    //---------------------------------------------------------------------------
    class LogPerThread
    {
        /// Internal Definition for Per-Transaction Log
    public:
        /// Fields for Log Entry
        std::deque<RecordId> recordIds; // Record Ids Accessed by This Transaction
        std::deque<Value> beforeValues; // Values Before All Operations Are Applied
        std::deque<Value> afterValues;  // Values After All Operations Are Applied

        /// File Output Interface
        std::ofstream fout;

        /// Default Structure
        LogPerThread()
        {
            // Do Nothing
        }
        /// Constructor for New Per-Transaction Log
        /// it just make new file stream object
        LogPerThread(TransactionId txnId) : fout("thread" + std::to_string(txnId) + ".txt")
        {
            // Do Nothing
        }
    };
    //---------------------------------------------------------------------------
    pthread_mutex_t globalMtx;                                 // Global Mutex for Protecting Execution Order
    uint64_t executionOrder;                                   // Current Execution Order
    std::unordered_map<TransactionId, LogPerThread> logPerTxn; // Per-Transaction Log Table

public:
    /// Constructor for Log Manager
    Logger(int64_t numTxn) : executionOrder(0)
    {
        // initialize global mutex
        globalMtx = PTHREAD_MUTEX_INITIALIZER;

        // initialize per-transaction log table
        for (TransactionId txnId = 1; txnId <= numTxn; txnId++)
        {
            logPerTxn[txnId] = LogPerThread(txnId);
        }
    }

    /// Add New Log Entry
    void addLog(TransactionId txnId, RecordId recordId, Value beforeValue, Value afterValue);
    /// Flush Log of Given Transaction
    void flushLog(uint64_t commitId, TransactionId txnId);
    /// Just Delete Log of Given Transaction
    void stashLog(TransactionId txnId);
    /// Get Special Log for UNDO Operation
    LogForUndo getLogForUndo(TransactionId txnId);
    /// Increment Execution Order and Return It Atomically
    uint64_t getCommitId();
};
//---------------------------------------------------------------------------
#endif