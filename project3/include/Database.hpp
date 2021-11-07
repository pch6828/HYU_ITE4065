#pragma once
#ifndef __DATABASE_HPP__
#define __DATABASE_HPP__

#include <vector>
#include "Defs.hpp"
#include "Lock.hpp"
#include "Logger.hpp"

enum State
{
    OK,
    CORRUPT,
    TERMINATED
};

class Database
{
private:
    LockTable lockTable;
    std::vector<Value> records;
    Logger logger;

public:
    Database(int64_t numRecord, int64_t numTxn, uint64_t maxExecution) : lockTable(numRecord, numTxn),
                                                                         records(numRecord + 1, 100),
                                                                         logger(numTxn);
    {
        // Do Nothing
    }

    State read(TransactionId txnId, RecordId recordId, Value &value);
    State add(TransactionId txnId, RecordId recordId, Value operand);
    void abort(TransactionId txnId);
    void commit(TransactionId txnId);
};

#endif