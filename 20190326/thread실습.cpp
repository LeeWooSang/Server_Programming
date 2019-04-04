#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
using namespace std;
using namespace chrono;

// volatile : ??? 
volatile int sum = 0;
mutex sum_lock;
static const int Thread_Size = 2;

void WorkerThread(int thread_id);
void make_sum();
void MultiThread_Test();
void SingleThread_Sum();
void MultiThread_Sum();

int main() 
{
	MultiThread_Test();
}

void WorkerThread(int thread_id)
{
	cout << "Thread ID : " << thread_id << endl;
}

void make_sum()
{
	volatile int local_sum = 0;
	// 스레드 개수만큼 일을 나눔
	for (int i = 0; i < 50000000 / Thread_Size; ++i)
	{
		local_sum = local_sum + 2;
	}
	sum_lock.lock();
	sum = sum + local_sum;
	sum_lock.unlock();
}

void MultiThread_Test()
{
	vector <thread> my_thread;
	auto start = high_resolution_clock::now();
	for (int i = 0; i < Thread_Size; ++i)
	{
		// thread 인자 값 : 
		// 첫번째 : 실행할 함수
		// 두번째 : 함수에 들어갈 인자 값
		my_thread.emplace_back(thread{ make_sum });
	}
	// 스레드 종료 기다리기
	for (auto& t : my_thread)
		t.join();
	auto elapsedTime = high_resolution_clock::now() - start;

	cout << "Sum = " << sum << ",  Time = " << duration_cast<milliseconds>(elapsedTime).count() << "밀리초" << endl;

	cout << "End of Threads" << endl;
}

void SingleThread_Sum()
{
	auto start = high_resolution_clock::now();
	for (auto i = 0; i < 50000000; ++i)
		sum += 2;
	auto elapsedTime = high_resolution_clock::now() - start;
	cout << "Sum = " << sum << ",  Time = " << duration_cast<milliseconds>(elapsedTime).count() << "밀리초" << endl;
}

void MultiThread_Sum()
{
	auto start = high_resolution_clock::now();

	thread t1{ make_sum };
	thread t2{ make_sum };
	t1.join();
	t2.join();

	auto elapsedTime = high_resolution_clock::now() - start;
	cout << "Sum = " << sum << ",  Time = " << duration_cast<milliseconds>(elapsedTime).count() << "밀리초" << endl;
}