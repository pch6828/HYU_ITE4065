#include <iostream>
#include <random>
#include <cstring>
#include "Snapshot.hpp"
#include "ThreadPool.hpp"

using namespace std;

Snapshot snapshot;
random_device rd;

void updateFunc(int tid)
{
    int value = rd();
    snapshot.update(tid, value);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Please, Give 1 Argument (# of Thread)." << endl;
        return 0;
    }

    int numThread = atoi(argv[1]);

    if (numThread <= 0)
    {
        cerr << "# of Thread Must Be Positive." << endl;
    }
    snapshot = Snapshot(numThread);
    ThreadPool threadPool(numThread, updateFunc);
    long numUpdate = threadPool.runAndChecksum(60);

    cout << numUpdate << endl;
    return 0;
}