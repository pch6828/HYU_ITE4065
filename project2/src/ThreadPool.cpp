#include "ThreadPool.hpp"
#include <pthread.h>
#include <thread>
#include <chrono>

using namespace std;
//---------------------------------------------------------------------------
void *threadFunc(void *arg)
{
    ThreadArg *threadArg = (ThreadArg *)arg;
    long localCount = 0;

    pthread_mutex_lock(threadArg->mtxForWorkers);
    threadArg->sleeping = true;
    pthread_cond_wait(threadArg->cvForWorkers, threadArg->mtxForWorkers);
    pthread_mutex_unlock(threadArg->mtxForWorkers);

    while (!(*(threadArg->terminateFlag)))
    {
        threadArg->work(threadArg->tid);
        localCount++;
    }

    return (void *)localCount;
}
//---------------------------------------------------------------------------
long ThreadPool::runAndChecksum(int runTime)
{
    pthread_mutex_lock(&(this->mtxForWorkers));
    pthread_cond_broadcast(&(this->cvForWorkers));
    pthread_mutex_unlock(&(this->mtxForWorkers));

    this_thread::sleep_for(chrono::seconds(runTime));
    this->terminateFlag = true;

    long checkSum = 0;

    for (int tid = 0; tid < this->numThread; tid++)
    {
        long localCount;
        pthread_join(workers[tid], (void **)(&localCount));
        checkSum += localCount;
    }

    return checkSum;
}
//---------------------------------------------------------------------------