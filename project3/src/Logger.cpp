#include "Logger.hpp"

using namespace std;
//---------------------------------------------------------------------------
/*
 * PAGE 1 : Function Definition for Log Manager
 */
void Logger::addLog(TransactionId txnId, RecordId recordId, Value beforeValue, Value afterValue)
// Add New Log Entry
{
    // get log of given transaction
    LogPerThread &myLog = logPerTxn[txnId];
    // add given new log entry
    myLog.recordIds.push_back(recordId);
    myLog.beforeValues.push_back(beforeValue);
    myLog.afterValues.push_back(afterValue);
}

void Logger::flushLog(uint64_t commitId, TransactionId txnId)
// Flush Log of Given Transaction
{
    // get log of given transaction
    LogPerThread &myLog = logPerTxn[txnId];

    // output given commit id
    myLog.fout << commitId << " ";
    // output all log entry
    for (RecordId recordId : myLog.recordIds)
    {
        myLog.fout << recordId << " ";
    }
    for (Value value : myLog.afterValues)
    {
        myLog.fout << value << " ";
    }
    // add new line character and flush file stream
    myLog.fout << endl;

    // delete all log entries
    myLog.recordIds.clear();
    myLog.beforeValues.clear();
    myLog.afterValues.clear();
}

void Logger::stashLog(TransactionId txnId)
// Just Delete Log of Given Transaction
{
    // get log of given transaction
    LogPerThread &myLog = logPerTxn[txnId];

    // delete all log entries
    myLog.recordIds.clear();
    myLog.beforeValues.clear();
    myLog.afterValues.clear();
}

LogForUndo Logger::getLogForUndo(TransactionId txnId)
// Get Special Log for UNDO Operation
{
    // get log of given transaction
    LogPerThread &myLog = logPerTxn[txnId];

    // make LogForUndo object and return it
    return LogForUndo(&(myLog.recordIds), &(myLog.beforeValues));
}

uint64_t Logger::getCommitId()
// Increment Execution Order and Return It Atomically
{
    uint64_t commitId;
    // acquire global mutex to protect atomicity
    pthread_mutex_lock(&globalMtx);
    // increment execution order
    executionOrder++;
    // fetch it into local variable
    commitId = executionOrder;
    // release global mutex
    pthread_mutex_unlock(&globalMtx);

    // return fetched value
    return commitId;
}
//---------------------------------------------------------------------------
