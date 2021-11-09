#pragma once
#ifndef __DATABASE_HPP__
#define __DATABASE_HPP__

#include <vector>
#include "Defs.hpp"
#include "Lock.hpp"
#include "Logger.hpp"
//---------------------------------------------------------------------------
/// Result State for Single Operation
enum State
{
    OK,        // Operation is Successed
    CORRUPT,   // Operation failed, it should restart
    TERMINATED // Total thread should be terminated
};
//---------------------------------------------------------------------------
class Database
{
    /// Definition for Database
    /// This is general interface for Database API
private:
    LockTable lockTable;        // Lock Table
    std::vector<Value> records; // Table for Real Records
    Logger logger;              // Log Manager
    uint64_t maxExecution;      // maximum execution order (flag for termination)

public:
    /// Constructor for Database
    Database(int64_t numRecord, int64_t numTxn, uint64_t maxExecution) : lockTable(numRecord, numTxn),
                                                                         records(numRecord + 1, 100),
                                                                         logger(numTxn),
                                                                         maxExecution(maxExecution)
    {
        // Do Nothing
    }

    /// Read Record
    State read(TransactionId txnId, RecordId recordId, Value &value);
    /// Add Value to Given Record
    State add(TransactionId txnId, RecordId recordId, Value operand);
    /// Stop Transaction and Rollback
    State abort(TransactionId txnId, bool terminated = false);
    /// Commit Transaction
    State commit(TransactionId txnId);
};
//---------------------------------------------------------------------------
#endif