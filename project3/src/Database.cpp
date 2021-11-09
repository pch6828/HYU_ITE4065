#include "Database.hpp"

using namespace std;
//---------------------------------------------------------------------------
/*
 * PAGE 1 : Function Definition for Database
 */
State Database::read(TransactionId txnId, RecordId recordId, Value &value)
// Read Record
{
    // try to acquire lock
    if (!lockTable.lock(txnId, recordId, READ))
    {
        // if there is deadlock,
        // read operation fails
        return CORRUPT;
    }

    // read record's value
    Value readValue = records[recordId];
    // add read log
    // since it does not change record's value, beforeValue and afterValue are same
    logger.addLog(txnId, recordId, readValue, readValue);
    // fetch operation's result
    value = readValue;

    // read operation succeeds
    return OK;
}

State Database::add(TransactionId txnId, RecordId recordId, Value operand)
// Add Value to Given Record
{
    // try to acquire lock
    if (!lockTable.lock(txnId, recordId, WRITE))
    {
        // if there is deadlock,
        // write operation fails
        return CORRUPT;
    }

    // fetch record's value
    Value beforeValue = records[recordId];
    // add value to record
    records[recordId] += operand;
    // fetch record's value again
    Value afterValue = records[recordId];
    // add write log
    logger.addLog(txnId, recordId, beforeValue, afterValue);

    // write operation succeeds
    return OK;
}

State Database::abort(TransactionId txnId, bool terminated)
// Stop Transaction and Rollback
{
    // get special log to recover
    LogForUndo logForUndo = logger.getLogForUndo(txnId);

    // for each operation that given transaction did,
    int numOperation = logForUndo.recordIds->size();
    for (int i = 0; i < numOperation; i++)
    {
        // get record id and value to be recovered
        RecordId recordId = (*(logForUndo.recordIds))[i];
        Value beforeValue = (*(logForUndo.beforeValues))[i];

        // reset record's value
        records[recordId] = beforeValue;
    }
    // throw away log
    logger.stashLog(txnId);
    // unlock all lock objects that given transaction acquired
    lockTable.unlock(txnId);
    // return result state
    return terminated ? TERMINATED : OK;
}

State Database::commit(TransactionId txnId)
// Commit Transaction
{
    // get commit id atomically
    uint64_t commitId = logger.getCommitId();

    // if commit id exceeds max execution number
    if (commitId > maxExecution)
    {
        // transaction should be aborted and rollbacked
        return abort(txnId, true);
    }

    // flush all log
    logger.flushLog(commitId, txnId);
    // unlock all lock objects that given transaction acquired
    lockTable.unlock(txnId);
    // return result state
    return OK;
}
//---------------------------------------------------------------------------