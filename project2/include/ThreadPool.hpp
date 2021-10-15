#pragma once
#include <pthread.h>
#include <valarray>
#include <iostream>
//---------------------------------------------------------------------------
struct ThreadArg
{
    int tid;
    bool sleeping;
    const bool *terminateFlag;
    pthread_mutex_t *mtxForWorkers;
    pthread_cond_t *cvForWorkers;
    void (*work)(int tid);

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

void *threadFunc(void *arg);
//---------------------------------------------------------------------------
class ThreadPool
{
private:
    int numThread;
    bool terminateFlag;

    pthread_mutex_t mtxForWorkers;
    pthread_cond_t cvForWorkers;
    std::valarray<pthread_t> workers;

public:
    ThreadPool(int numThread, int runTime, void (*work)(int tid)) : numThread(numThread),
                                                                    terminateFlag(false),
                                                                    workers(std::valarray<pthread_t>(numThread))
    {
        this->mtxForWorkers = PTHREAD_MUTEX_INITIALIZER;
        this->cvForWorkers = PTHREAD_COND_INITIALIZER;
        for (int tid = 0; tid < numThread; tid++)
        {
            ThreadArg *threadArg = new ThreadArg(tid,
                                                 &(this->terminateFlag),
                                                 work,
                                                 &(this->mtxForWorkers),
                                                 &(this->cvForWorkers));

            if (pthread_create(&workers[tid], 0, threadFunc, (void *)threadArg) < 0)
            {
                std::cerr << "Thread Allocation Failed" << std::endl;
            }

            while (!(threadArg->sleeping))
            {
                pthread_yield();
            }
        }
    }

    ~ThreadPool()
    {
        if (!(this->terminateFlag))
        {
            this->terminateFlag = true;
            for (int tid = 0; tid < this->numThread; tid++)
            {
                pthread_join(workers[tid], NULL);
            }
        }
    }

    long runAndChecksum(int runTime);
};
//---------------------------------------------------------------------------