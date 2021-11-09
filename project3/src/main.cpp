#include <iostream>
#include <random>
#include <cstring>
#include <cmath>
#include "Defs.hpp"
#include "Database.hpp"
#include "ThreadPool.hpp"

using namespace std;

// database object
Database *db;
// random device for make random integer
random_device rd;
// argument given by command line
int64_t numThread;
int64_t numRecord;
int64_t maxExecution;

bool transaction(TransactionId txnId)
// work function for thread pool
{
    // get random, unique integers by using random device
    RecordId i, j, k;
    i = rd() % numRecord + 1;
    j = rd() % numRecord + 1;
    while (j == i)
    {
        j = rd() % numRecord + 1;
    }
    k = rd() % numRecord + 1;
    while (k == j || k == i)
    {
        k = rd() % numRecord + 1;
    }

    // repeat loop until transaction commits
    while (true)
    {
        Value record;
        State state;

        // try read operation
        state = db->read(txnId, i, record);
        // if corrunted, abort
        if (state == CORRUPT)
        {
            db->abort(txnId);
            continue;
        }

        // try write operation
        state = db->add(txnId, j, record + 1);
        // if corrunted, abort
        if (state == CORRUPT)
        {
            db->abort(txnId);
            continue;
        }

        // try write operation
        state = db->add(txnId, k, -record);
        // if corrunted, abort
        if (state == CORRUPT)
        {
            db->abort(txnId);
            continue;
        }

        // commit transaction
        state = db->commit(txnId);
        // return whether thread should be terminated or not
        return state == TERMINATED;
    }

    // dummy statement, just for avoiding compile error
    return false;
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cerr << "Please, Give 3 Argument (# of Thread, # of Records, Max Execution)." << endl;
        return 0;
    }

    // # of threads given by command line argument
    numThread = atoi(argv[1]);
    // # of records given by command line argument
    numRecord = atoi(argv[2]);
    // maximum execution number given by command line argument
    maxExecution = atoi(argv[3]);

    if (numThread <= 0)
    {
        cerr << "# of Thread Must Be Positive." << endl;
        return 0;
    }
    if (numRecord < 3)
    {
        cerr << "# of Record Must Be Greater Or Equal Than 3." << endl;
        return 0;
    }
    if (maxExecution <= 0)
    {
        cerr << "Maximum Execution Must Be Positive." << endl;
        return 0;
    }

    // make database instance
    db = new Database(numRecord, numThread, maxExecution);
    // make thread pool
    ThreadPool threadPool(numThread, transaction);
    // run all threads in thread pool
    threadPool.run();

    // deallocate snapshot instance
    delete db;
    return 0;
}