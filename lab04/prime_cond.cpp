#include <stdio.h>
#include <pthread.h>
#include <math.h>

#define NUM_THREAD 10

int thread_ret[NUM_THREAD];
int done = 0;
bool is_done = false;

int range_start;
int range_end;

pthread_mutex_t mtx_for_workers = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv_for_workers = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mtx_for_main = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv_for_main = PTHREAD_COND_INITIALIZER;

bool IsPrime(int n)
{
    if (n < 2)
    {
        return false;
    }

    for (int i = 2; i <= sqrt(n); i++)
    {
        if (n % i == 0)
        {
            return false;
        }
    }
    return true;
}

void *ThreadFunc(void *arg)
{
    long tid = (long)arg;
    pthread_mutex_lock(&mtx_for_workers);
    thread_ret[tid] = -1;
    pthread_cond_wait(&cv_for_workers, &mtx_for_workers);
    pthread_mutex_unlock(&mtx_for_workers);
    while (!is_done)
    {
        // Split range for this thread
        int start = range_start + ((range_end - range_start + 1) / NUM_THREAD) * tid;
        int end = range_start + ((range_end - range_start + 1) / NUM_THREAD) * (tid + 1);
        if (tid == NUM_THREAD - 1)
        {
            end = range_end + 1;
        }

        long cnt_prime = 0;
        for (int i = start; i < end; i++)
        {
            if (IsPrime(i))
            {
                cnt_prime++;
            }
        }

        thread_ret[tid] = cnt_prime;
        __sync_fetch_and_add(&done, 1);

        pthread_mutex_lock(&mtx_for_workers);
        pthread_cond_wait(&cv_for_workers, &mtx_for_workers);
        pthread_mutex_unlock(&mtx_for_workers);
    }
    return NULL;
}

int main(void)
{
    pthread_t threads[NUM_THREAD];
    // Create threads to work
    for (long i = 0; i < NUM_THREAD; i++)
    {
        if (pthread_create(&threads[i], 0, ThreadFunc, (void *)i) < 0)
        {
            printf("pthread_create error!\n");
            return 0;
        }
        while (thread_ret[i] != -1)
        {
            pthread_yield();
        }
    }

    while (1)
    {
        // Input range
        scanf("%d", &range_start);
        if (range_start == -1)
        {
            is_done = true;
            pthread_mutex_lock(&mtx_for_workers);
            pthread_cond_broadcast(&cv_for_workers);
            pthread_mutex_unlock(&mtx_for_workers);
            break;
        }
        scanf("%d", &range_end);

        done = 0;

        pthread_mutex_lock(&mtx_for_workers);
        pthread_cond_broadcast(&cv_for_workers);
        pthread_mutex_unlock(&mtx_for_workers);

        while (done != NUM_THREAD)
        {
            pthread_yield();
        }

        // Collect results
        int cnt_prime = 0;
        for (int i = 0; i < NUM_THREAD; i++)
        {
            cnt_prime += thread_ret[i];
        }
        printf("number of prime: %d\n", cnt_prime);
    }

    // Wait threads end
    for (int i = 0; i < NUM_THREAD; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
