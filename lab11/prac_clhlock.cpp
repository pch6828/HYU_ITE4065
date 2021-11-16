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

	int *lock()
	{
		int *myNode = new int(1);
		int *pred = __sync_lock_test_and_set(&tail, myNode);
		while (*pred)
		{
			// wait
		}
		return myNode;
	}

	void unlock(int *myNode)
	{
		*myNode = 0;
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
		myNode = object_clh.lock();
		cnt_global++;
		printf("%d\n", cnt_global);
		object_clh.unlock(myNode);
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
