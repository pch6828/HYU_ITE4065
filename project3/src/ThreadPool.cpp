#include "ThreadPool.hpp"
#include <pthread.h>
#include <thread>
#include <chrono>

using namespace std;
//---------------------------------------------------------------------------
/*
 * PAGE 1 : General Thread Function
 **/
void *threadFunc(void *arg)
// General Thread Function
// arg will be ThreadArg*, and this will be used in pthread_create().
{
    // type-cast given argument
    ThreadArg *threadArg = (ThreadArg *)arg;

    // sleep for signal of main thread
    pthread_mutex_lock(threadArg->mtxForWorkers);
    // set its sleeping flag
    threadArg->sleeping = true;
    pthread_cond_wait(threadArg->cvForWorkers, threadArg->mtxForWorkers);
    pthread_mutex_unlock(threadArg->mtxForWorkers);

    // infinite loop until main thread indicates program's termination.
    while (!(*(threadArg->terminateFlag)))
    {
        // call given work function
        if (threadArg->work(threadArg->txnId))
        {
            break;
        }
    }
    // deallocate given argument
    delete threadArg;
    // return local count
    return nullptr;
}
//---------------------------------------------------------------------------
/*
 * PAGE 2 : Function Definition for ThreadPool
 **/
void ThreadPool::run()
// Run All Threads at the Same Time, And Return Sum of Their Results
{
    // wake up all threads
    pthread_mutex_lock(&(this->mtxForWorkers));
    pthread_cond_broadcast(&(this->cvForWorkers));
    pthread_mutex_unlock(&(this->mtxForWorkers));

    for (int tid = 0; tid < this->numThread; tid++)
    {
        // wait thread finish
        pthread_join(workers[tid], nullptr);
    }
    // since run time has passed,
    // set termination flag to make all threads stop infinite loop.
    this->terminateFlag = true;
}
//---------------------------------------------------------------------------