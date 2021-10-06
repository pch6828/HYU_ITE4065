#include <iostream>
#include <vector>
#include <pthread.h>
#include "Joiner.hpp"
#include "Parser.hpp"

using namespace std;
//---------------------------------------------------------------------------
#define NUM_THREAD 40
//---------------------------------------------------------------------------
Joiner joiner;
bool is_done = false;
uint64_t num_active_thread = 0;
QueryInfo *thread_arg[NUM_THREAD];
string *volatile thread_ret[NUM_THREAD];
pthread_t threads[NUM_THREAD];
pthread_mutex_t mtx_for_workers[NUM_THREAD];
pthread_cond_t cv_for_workers[NUM_THREAD];
//---------------------------------------------------------------------------
void *thread_func(void *arg)
{
   long tid = (long)arg;
   pthread_mutex_lock(&mtx_for_workers[tid]);
   thread_ret[tid] = (string *)-1;
   pthread_cond_wait(&cv_for_workers[tid], &mtx_for_workers[tid]);
   pthread_mutex_unlock(&mtx_for_workers[tid]);

   while (!is_done)
   {
      QueryInfo &i = *(thread_arg[tid]);

      string *result = new string(joiner.join(i));

      pthread_mutex_lock(&mtx_for_workers[tid]);
      thread_ret[tid] = result;
      pthread_cond_wait(&cv_for_workers[tid], &mtx_for_workers[tid]);
      pthread_mutex_unlock(&mtx_for_workers[tid]);
   }

   return nullptr;
}

void flush_all_thread()
{
   for (long i = 0; i < num_active_thread; i++)
   {
      while (thread_ret[i] == (string *)-1)
      {
         pthread_yield();
      }
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
   //

   for (long i = 0; i < NUM_THREAD; i++)
   {
      mtx_for_workers[i] = PTHREAD_MUTEX_INITIALIZER;
      cv_for_workers[i] = PTHREAD_COND_INITIALIZER;
      if (pthread_create(&threads[i], 0, thread_func, (void *)i) < 0)
      {
         printf("pthread_create error!\n");
         return 0;
      }
      while (thread_ret[i] != (string *)-1)
      {
         pthread_yield();
      }
   }

   while (getline(cin, line))
   {
      if (line == "F")
      {
         flush_all_thread();
         num_active_thread = 0;
         continue;
      }

      QueryInfo *i = new QueryInfo();
      i->parseQuery(line);
      if (num_active_thread == NUM_THREAD)
      {
         flush_all_thread();
         num_active_thread = 0;
      }
      thread_arg[num_active_thread] = i;
      thread_ret[num_active_thread] = (string *)-1;
      pthread_mutex_lock(&mtx_for_workers[num_active_thread]);
      pthread_cond_broadcast(&cv_for_workers[num_active_thread]);
      pthread_mutex_unlock(&mtx_for_workers[num_active_thread]);

      num_active_thread++;
   }

   // Wait threads end
   is_done = true;
   for (int i = 0; i < NUM_THREAD; i++)
   {
      pthread_mutex_lock(&mtx_for_workers[i]);
      pthread_cond_broadcast(&cv_for_workers[i]);
      pthread_mutex_unlock(&mtx_for_workers[i]);
      pthread_join(threads[i], NULL);
   }

   return 0;
}
