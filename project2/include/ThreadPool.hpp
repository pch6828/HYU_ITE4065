#pragma once
#include <pthread.h>
#include <valarray>
#include <iostream>
//---------------------------------------------------------------------------
struct ThreadArg
{
    /// Definition for Pthread Arguments

    /// Thread ID
    int tid;
    /// Flag for Indicating Whether Thread is Sleeping or Not
    /// it is used to make thread sleep until workload is given.
    bool sleeping;
    /// Flag to Stop Thread's Infinite Loop
    const bool *terminateFlag;
    /// Function Pointer of Workload
    /// it'll be called repeatedly until terminateFlag is set.
    void (*work)(int tid);
    /// Mutex and Condition Variable to Make Thread Sleep and Wake Up
    /// these are used to make thread sleep when created,
    /// and start all at the same time.
    pthread_mutex_t *mtxForWorkers;
    pthread_cond_t *cvForWorkers;

    /// Constructor
    ThreadArg(int tid,
              const bool *terminateFlag,
              void (*work)(int tid),
              pthread_mutex_t *mtxForWorkers,
              pthread_cond_t *cvForWorkers) : tid(tid),
                                              sleeping(false),
                                              terminateFlag(terminateFlag),
                                              work(work),
                                              mtxForWorkers(mtxForWorkers),
                                              cvForWorkers(cvForWorkers)
    {
        //Do Nothing
    }
};

/// General Thread Function
/// arg will be ThreadArg*, and this will be used in pthread_create().
void *threadFunc(void *arg);
//---------------------------------------------------------------------------
class ThreadPool
{
    /// Definition for ThreadPool
private:
    /// # of Thread in ThreadPool
    int numThread;
    /// Flag for Termination of All thread
    /// address of this value will be used in ThreadArg's initialization.
    bool terminateFlag;
    /// Mutex and Condition Variable to Make Thread Sleep and Wake Up
    /// these are used to make thread sleep when created,
    /// and start all at the same time.
    /// address of these will be used in ThreadArg's initialization. (All threads share same mutex and condition variable)
    pthread_mutex_t mtxForWorkers;
    pthread_cond_t cvForWorkers;
    /// Actual Threads in ThreadPool
    std::valarray<pthread_t> workers;

public:
    /// Constructor
    ThreadPool(int numThread, void (*work)(int tid)) : numThread(numThread),
                                                       terminateFlag(false),
                                                       workers(std::valarray<pthread_t>(numThread))
    {
        // initialize mutex and condition_variable
        this->mtxForWorkers = PTHREAD_MUTEX_INITIALIZER;
        this->cvForWorkers = PTHREAD_COND_INITIALIZER;
        for (int tid = 0; tid < numThread; tid++)
        {
            // make argument for each thread
            ThreadArg *threadArg = new ThreadArg(tid,
                                                 &(this->terminateFlag),
                                                 work,
                                                 &(this->mtxForWorkers),
                                                 &(this->cvForWorkers));

            // create thread with threadArg and threadFunc
            if (pthread_create(&workers[tid], 0, threadFunc, (void *)threadArg) < 0)
            {
                std::cerr << "Thread Allocation Failed" << std::endl;
            }
            // spin-wait for thread's sleep
            // all thread will start processing at the same time by pthread_broadcast
            while (!(threadArg->sleeping))
            {
                pthread_yield();
            }
        }
    }

    /// Destructor
    ~ThreadPool()
    {
        // if termination flag is not set,
        // it means every thread is waiting for broadcast,
        if (!(this->terminateFlag))
        {
            // set termination flag
            this->terminateFlag = true;
            // wake up all threads
            pthread_cond_broadcast(&cvForWorkers);

            // wait each thread finish
            for (int tid = 0; tid < this->numThread; tid++)
            {
                pthread_join(workers[tid], NULL);
            }
        }
    }

    /// Run All Threads at the Same Time, And Return Sum of Their Results
    long runAndChecksum(int runTime);
};
//---------------------------------------------------------------------------