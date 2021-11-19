#include <stdio.h>
#include <pthread.h>
#include <memory>

#define NUM_THREAD 16
#define NUM_WORK 1000000

class CLHLock
{
private:
	int *tail;

public:
	CLHLock()
	{
		tail = new int(0);
	}
	~CLHLock()
	{
		delete tail;
	}

	int *lock(int *&myNode)
	{
		*myNode = 1;
		int *pred = __sync_lock_test_and_set(&tail, myNode);
		while (*pred)
		{
			// wait
		}
		return pred;
	}

	void unlock(int *&myNode, int *pred)
	{
		*myNode = 0;
		myNode = pred;
		__sync_synchronize();
	}
};

int cnt_global;
/* to allocate cnt_global & object_tas in different cache lines */
int gap[128];
CLHLock object_clh;

void *thread_work(void *args)
{
	int *myNode = new int(1);
	for (int i = 0; i < NUM_WORK; i++)
	{
		int *pred = object_clh.lock(myNode);
		cnt_global++;
		object_clh.unlock(myNode, pred);
	}
}

int main(void)
{
	pthread_t threads[NUM_THREAD];

	for (int i = 0; i < NUM_THREAD; i++)
	{
		pthread_create(&threads[i], NULL, thread_work, NULL);
	}
	for (int i = 0; i < NUM_THREAD; i++)
	{
		pthread_join(threads[i], NULL);
	}
	printf("cnt_global: %d\n", cnt_global);

	return 0;
}
