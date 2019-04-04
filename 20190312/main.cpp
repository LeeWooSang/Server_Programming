#include <iostream>
#include <chrono>
#include <thread>
#include <Windows.h>

using namespace std;
using namespace chrono;

void SystenCall();
void CacheMiss();
static long abs(long x);
void BranchMiss();
int main()
{

}
void SystenCall()
{
	volatile long long tmp = 0;
	auto start = chrono::high_resolution_clock::now();
	for (int i = 0; i < 10000000; ++i)
	{
		tmp += i;
		this_thread::yield();
	}
	auto elapsedTime = chrono::high_resolution_clock::now() - start;

	cout << "Time : " << chrono::duration_cast<chrono::milliseconds>(elapsedTime).count() << " 밀리초" << endl;
	cout << "tmp = " << tmp << endl;
}

void CacheMiss()
{

	for (int i = 0; i < 21; ++i) {
		int size = 1024 << i;
		char *a = (char *)malloc(size);
		unsigned int index = 0;
		volatile unsigned int tmp = 0;
		auto start = chrono::high_resolution_clock::now();
		for (int j = 0; j < 100000000; ++j) {
			tmp += a[index % size];
			//index += CACHE_LINE_SIZE * 11;
		}
		auto dur = chrono::high_resolution_clock::now() - start;
	}

}
// 절대값 구하는 매크로
#define abs(x) (((x)>0)?(x):-(x))
#define T_SIZE 10000000
// 절대값 구하는 함수
static long ABS(long x) 
{
	long y;
	y = x >> 31; /* Not portable */
	return (x^y) - y;
}

void BranchMiss()
{
	//int sum;
	//for (int i = 0; i < T_SIZE; ++i) rand_arr[i] = rand() - 16384;
	//sum = 0;
	//auto start_t = high_resolution_clock::now();
	//for (int i = 0; i < T_SIZE; ++i) sum += abs(rand_arr[i]);
	//auto du = high_resolution_clock::now() - start_t;
	//cout << "[abs] Time " << duration_cast<milliseconds>(du).count() << " ms\n";
	//cout << "Result : " << sum << endl;
	//sum = 0;
	//start_t = high_resolution_clock::now();
	//for (int i = 0; i < T_SIZE; ++i) sum += abs2(rand_arr[i]);
	//du = high_resolution_clock::now() - start_t;
	//cout << "[abs2] Time " << duration_cast<milliseconds>(du).count() << " ms\n";
	//cout << "Result : " << sum << endl;

}