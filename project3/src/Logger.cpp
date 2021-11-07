#include "Logger.hpp"

using namespace std;

void Logger::addLog(TransactionId txnId, RecordId recordId, Value beforeValue, Value afterValue)
{
    LogPerThread &myLog = logPerTxn[txnId];
    myLog.recordIds.push_back(recordId);
    myLog.beforeValues.push_back(beforeValue);
    myLog.afterValues.push_back(afterValue);
}

void Logger::flushLog(uint64_t commitId, TransactionId txnId)
{
    LogPerThread &myLog = logPerTxn[txnId];

    myLog.fout << commitId << " ";
    for (RecordId recordId : myLog.recordIds)
    {
        myLog.fout << recordId << " ";
    }
    for (Value value : myLog.afterValues)
    {
        myLog.fout << value << " ";
    }
    myLog.fout << endl;

    myLog.recordIds.clear();
    myLog.beforeValues.clear();
    myLog.afterValues.clear();
}

void Logger::stashLog(TransactionId txnId)
{
    LogPerThread &myLog = logPerTxn[txnId];

    myLog.recordIds.clear();
    myLog.beforeValues.clear();
    myLog.afterValues.clear();
}

LogForUndo Logger::getLogForUndo(TransactionId txnId)
{
    LogPerThread &myLog = logPerTxn[txnId];

    return LogForUndo(&(myLog.recordIds), &(myLog.beforeValues));
}

uint64_t Logger::getCommitId()
{
    uint64_t commitId;
    pthread_mutex_lock(&globalMtx);
    executionOrder++;
    commitId = executionOrder;
    pthread_mutex_unlock(&globalMtx);

    return commitId;
}