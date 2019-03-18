#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include<condition_variable>
using namespace std;

mutex mtx; //mutex to lock queue access
mutex prnt; //mutex to ensure print statements are not jumbled
condition_variable cnd; //condition variable to signal thread
queue<int> ndata; //queue in which producer will produce data
int n = 30000;

//Single producer, put a positive integer in queue every 100 ms
void producer()
{
	for (int i = 1; i <= n; i++)
	{
		ndata.push(i);
		this_thread::sleep_for(chrono::milliseconds(100));
		cnd.notify_one(); //notify one of the thread to process it
	}
}

//get element from queue. If queue empty, then returnes -1
int get_data()
{
	lock_guard<mutex> grd(mtx); //to make sure lock is released even after exception
	if (!ndata.empty())
	{
		int tmp = ndata.front();
		ndata.pop();
		return tmp;
	}
	else
		return -1;
}

//multi consumers. Time taken by each consumer to process data entry is 1 second.
void consumer(int id)
{
	while (true)
	{
		int d = get_data();
		if (d == -1)
		{
			unique_lock<mutex> ulk(mtx);
			cnd.wait(ulk); //queue is empty, wait for signal
			continue;
		}
		this_thread::sleep_for(chrono::milliseconds(1000)); //sleep to simulate data processing
		lock_guard<mutex> lgprnt(prnt); //make sure print mutex released even after exception
		cout << "Consumer ID = " << id << " Processing complete for data = " << d << endl << endl;
	}
}

int main()
{
	thread p(producer); // 1 producer thread
	thread *c[5]; //Total consumer threads 5
	for (int i = 0; i < 5; i++)
	{
		c[i] = new thread(consumer, i + 1);
	}

	//joining all threads.
	for (int i = 0; i < 5; i++)
	{
		c[i]->join();
	}
	p.join();

	return 0;
}