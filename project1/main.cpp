#include <iostream>
#include <vector>
#include <pthread.h>
#include "Joiner.hpp"
#include "Parser.hpp"

using namespace std;
//---------------------------------------------------------------------------
Joiner joiner;
//---------------------------------------------------------------------------

void *thread_func(void *arg)
{
   QueryInfo &i = *((QueryInfo *)arg);
   string *result = new string(joiner.join(i));
   pthread_exit((void *)result);
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
   vector<pthread_t *> threads;
   while (getline(cin, line))
   {
      if (line == "F")
      {
         for (auto &thread : threads)
         {
            void *ret;
            pthread_join(*thread, &ret);
            cout << *((string *)ret);
         }
         threads.clear();
         continue;
      }

      pthread_t *thread = new pthread_t();
      threads.push_back(thread);

      QueryInfo *i = new QueryInfo();
      i->parseQuery(line);
      if (pthread_create(thread, NULL, thread_func, i) < 0)
      {
         exit(-1);
      }
   }

   return 0;
}
