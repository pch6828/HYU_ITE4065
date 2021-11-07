#pragma once
#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <fstream>
#include <string>
#include <deque>
#include <unordered_map>
#include <pthread.h>
#include "Defs.hpp"

class LogForUndo
{
public:
    const std::deque<RecordId> *recordIds;
    const std::deque<Value> *beforeValues;
    LogForUndo(const std::deque<RecordId> *recordIds,
               const std::deque<Value> *beforeValues) : recordIds(recordIds),
                                                        beforeValues(beforeValues)
    {
        // Do Nothing
    }
};

class Logger
{
private:
    class LogPerThread
    {
    public:
        std::deque<RecordId> recordIds;
        std::deque<Value> beforeValues;
        std::deque<Value> afterValues;

        std::ofstream fout;

        LogPerThread()
        {
            // Do Nothing
        }
        LogPerThread(TransactionId txnId) : fout("thread" + std::to_string(txnId) + ".txt")
        {
            // Do Nothing
        }
    };

    pthread_mutex_t globalMtx;
    uint64_t executionOrder;
    std::unordered_map<TransactionId, LogPerThread> logPerTxn;

public:
    Logger(int64_t numTxn) : executionOrder(0)
    {
        globalMtx = PTHREAD_MUTEX_INITIALIZER;

        for (TransactionId txnId = 1; txnId <= numTxn; txnId++)
        {
            logPerTxn[txnId] = LogPerThread(txnId);
        }
    }

    void addLog(TransactionId txnId, RecordId recordId, Value beforeValue, Value afterValue);
    void flushLog(uint64_t commitId, TransactionId txnId);
    void stashLog(TransactionId txnId);
    LogForUndo getLogForUndo(TransactionId txnId);
    uint64_t getCommitId();
};
#endif