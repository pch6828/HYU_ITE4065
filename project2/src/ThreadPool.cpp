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
    // set its local count to 0
    long localCount = 0;

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
        // in this program, work function will be snapshot update.
        threadArg->work(threadArg->tid);
        // increment local count
        // in this program, this means # of updates done by this thread.
        localCount++;
    }
    // deallocate given argument
    delete threadArg;
    // return local count
    return (void *)localCount;
}
//---------------------------------------------------------------------------
/* 
 * PAGE 2 : Function Definition for ThreadPool
**/
long ThreadPool::runAndChecksum(int runTime)
// Run All Threads at the Same Time, And Return Sum of Their Results
{
    // wake up all threads
    pthread_mutex_lock(&(this->mtxForWorkers));
    pthread_cond_broadcast(&(this->cvForWorkers));
    pthread_mutex_unlock(&(this->mtxForWorkers));

    // sleep for given run time
    // in this program, it'll be 60 seconds.
    this_thread::sleep_for(chrono::seconds(runTime));
    // since run time has passed,
    // set termination flag to make all threads stop infinite loop.
    this->terminateFlag = true;

    // total result
    long checkSum = 0;

    for (int tid = 0; tid < this->numThread; tid++)
    {
        long localCount;
        // wait thread finish, and get its return value
        pthread_join(workers[tid], (void **)(&localCount));
        // add each thread's local result to total result
        checkSum += localCount;
    }

    // return total result
    return checkSum;
}
//---------------------------------------------------------------------------