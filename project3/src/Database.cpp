#include "Database.hpp"

using namespace std;

State Database::read(TransactionId txnId, RecordId recordId, Value &value)
{
    if (!lockTable.lock(txnId, recordId, READ))
    {
        return CORRUPT;
    }

    Value readValue = records[recordId];
    logger.addLog(txnId, recordId, readValue, readValue);
    value = readValue;

    return OK;
}

State Database::add(TransactionId txnId, RecordId recordId, Value operand)
{
    if (!lockTable.lock(txnId, recordId, WRITE))
    {
        return CORRUPT;
    }

    Value beforeValue = records[recordId];
    records[recordId] += operand;
    Value afterValue = records[recordId];
    logger.addLog(txnId, recordId, beforeValue, afterValue);

    return OK;
}

State Database::abort(TransactionId txnId, bool terminated)
{
    LogForUndo logForUndo = logger.getLogForUndo(txnId);
    int numOperation = logForUndo.recordIds->size();
    for (int i = 0; i < numOperation; i++)
    {
        RecordId recordId = (*(logForUndo.recordIds))[i];
        Value beforeValue = (*(logForUndo.beforeValues))[i];

        records[recordId] = beforeValue;
    }
    logger.stashLog(txnId);
    lockTable.unlock(txnId);
    return terminated ? TERMINATED : OK;
}

State Database::commit(TransactionId txnId)
{
    uint64_t commitId = logger.getCommitId();
    if (commitId > maxExecution)
    {
        return abort(txnId, true);
    }

    logger.flushLog(commitId, txnId);
    lockTable.unlock(txnId);
    return OK;
}