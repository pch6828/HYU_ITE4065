#include <iostream>
#include <random>
#include <cstring>
#include <cmath>
#include "Defs.hpp"
#include "Database.hpp"
#include "ThreadPool.hpp"

using namespace std;

Database *db;
// random device for make random integer
random_device rd;
int64_t numThread;
int64_t numRecord;
int64_t maxExecution;

bool transaction(TransactionId txnId)
// work function for thread pool
{
    // get random integer by using random device
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

    while (true)
    {
        Value record;
        State state;

        state = db->read(txnId, i, record);
        if (state == CORRUPT)
        {
            db->abort(txnId);
            continue;
        }

        state = db->add(txnId, j, record + 1);
        if (state == CORRUPT)
        {
            db->abort(txnId);
            continue;
        }

        state = db->add(txnId, k, -record);
        if (state == CORRUPT)
        {
            db->abort(txnId);
            continue;
        }

        state = db->commit(txnId);
        return state == TERMINATED;
    }

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
    numRecord = atoi(argv[2]);
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