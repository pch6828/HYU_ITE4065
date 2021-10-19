#include <iostream>
#include <random>
#include <cstring>
#include "Snapshot.hpp"
#include "ThreadPool.hpp"

using namespace std;

Snapshot *snapshot;
// random device for make random integer
random_device rd;

void updateFunc(int tid)
// work function for thread pool
{
    // get random integer by using random device
    int value = rd();
    // update snapshot
    snapshot->update(tid, value);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Please, Give 1 Argument (# of Thread)." << endl;
        return 0;
    }

    // # of threads given by command line argument
    int numThread = atoi(argv[1]);

    if (numThread <= 0)
    {
        cerr << "# of Thread Must Be Positive." << endl;
    }

    // make snapshot instance
    snapshot = new Snapshot(numThread);
    // make thread pool
    ThreadPool threadPool(numThread, updateFunc);
    // run all threads in thread pool for 60 seconds
    long numUpdate = threadPool.runAndChecksum(60);

    // deallocate snapshot instance
    delete snapshot;
    // print out # of updates to stdout
    cout << numUpdate << endl;
    return 0;
}