#include <iostream>
#include <vector>
#include <pthread.h>
#include "Joiner.hpp"
#include "Parser.hpp"

using namespace std;
//---------------------------------------------------------------------------
#define NUM_THREAD 47
//---------------------------------------------------------------------------
Joiner joiner;
// flag value to indicate program's termination
bool is_done = false;
// number of active thread
// this value is used at waiting each thread's result.
// Since each batch's size varies, it need this value.
uint64_t num_active_thread = 0;
// array for store each thread's argument value
QueryInfo *thread_arg[NUM_THREAD];
// array for store each thread's return value
// to avoid compiler's optimization, it is declared as volatile value.
string *volatile thread_ret[NUM_THREAD];
// thread array which is used as basic thread pool
pthread_t threads[NUM_THREAD];
// since my structure is running each query in parallel
// and each query is completely separated
// it needs mutexes and condition variables with the same number of threads.
pthread_mutex_t mtx_for_workers[NUM_THREAD];
pthread_cond_t cv_for_workers[NUM_THREAD];
//---------------------------------------------------------------------------
void *thread_func(void *arg)
// thread function for pthread
{
   // thread id is given as argument.
   long tid = (long)arg;

   // wait for first input
   pthread_mutex_lock(&mtx_for_workers[tid]);
   // set flag to -1
   // -1 means this thread is sleeping
   thread_ret[tid] = (string *)-1;
   pthread_cond_wait(&cv_for_workers[tid], &mtx_for_workers[tid]);
   pthread_mutex_unlock(&mtx_for_workers[tid]);

   // loop until entire program's termination
   while (!is_done)
   {
      // each work is passed by global array
      QueryInfo &i = *(thread_arg[tid]);

      // run join operation and get its value.
      string *result = new string(joiner.join(i));

      // wait for next input
      pthread_mutex_lock(&mtx_for_workers[tid]);
      // save its result in global array
      // this also means, this thread's work is ended, and it need to be flushed.
      thread_ret[tid] = result;
      pthread_cond_wait(&cv_for_workers[tid], &mtx_for_workers[tid]);
      pthread_mutex_unlock(&mtx_for_workers[tid]);
   }

   return nullptr;
}
//---------------------------------------------------------------------------
void flush_all_thread()
/// wait for results from all threads, and flush it.
{
   // for all active threads,
   for (long i = 0; i < num_active_thread; i++)
   {
      // wait for each thread's result.
      while (thread_ret[i] == (string *)-1)
      {
         pthread_yield();
      }
      // print it out and set flag to -1;
      cout << *(thread_ret[i]);
      thread_ret[i] = (string *)-1;
   }
}
//---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
   // Read join relations
   string line;
   while (getline(cin, line))
   {
      if (line == "Done")
         break;
      joiner.addRelation(line.c_str());
   }
   // Preparation phase (not timed)
   // Build histograms, indexes,...

   // initialize thread pool
   for (long i = 0; i < NUM_THREAD; i++)
   {
      // initialize each thread's mutex and condition variable
      mtx_for_workers[i] = PTHREAD_MUTEX_INITIALIZER;
      cv_for_workers[i] = PTHREAD_COND_INITIALIZER;

      // run each thread
      if (pthread_create(&threads[i], 0, thread_func, (void *)i) < 0)
      {
         printf("pthread_create error!\n");
         return 0;
      }

      // spin wait until each thread wait for new input.
      while (thread_ret[i] != (string *)-1)
      {
         pthread_yield();
      }
   }

   while (getline(cin, line))
   {
      // if single batch is ended.
      // flush all threads' result
      // after flush, number of active thread should be 0.
      if (line == "F")
      {
         flush_all_thread();
         num_active_thread = 0;
         continue;
      }

      // make new argument for next thread
      // by parsing input query
      QueryInfo *i = new QueryInfo();
      i->parseQuery(line);

      // if thread pool does not have sleeping thread
      // then flush all threads' result
      // after flush, number of active thread should be 0.
      if (num_active_thread == NUM_THREAD)
      {
         flush_all_thread();
         num_active_thread = 0;
      }

      // pass argument to thread
      // and set its return value as -1 (because it is also used as flag for end of thread's single work.)
      thread_arg[num_active_thread] = i;
      thread_ret[num_active_thread] = (string *)-1;

      // wake up single thread
      pthread_mutex_lock(&mtx_for_workers[num_active_thread]);
      pthread_cond_signal(&cv_for_workers[num_active_thread]);
      pthread_mutex_unlock(&mtx_for_workers[num_active_thread]);

      // since new thread start working,
      // number of active thread should be incremented.
      num_active_thread++;
   }

   // set termination flag to true.
   is_done = true;
   // wake up each thread and wait for its return.
   for (int i = 0; i < NUM_THREAD; i++)
   {
      pthread_mutex_lock(&mtx_for_workers[i]);
      pthread_cond_signal(&cv_for_workers[i]);
      pthread_mutex_unlock(&mtx_for_workers[i]);
      pthread_join(threads[i], NULL);
   }

   return 0;
}
