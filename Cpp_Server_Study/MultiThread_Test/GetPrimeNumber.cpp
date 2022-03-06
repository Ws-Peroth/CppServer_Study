#include <list>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>

using namespace std;

const int MaxCount = 150000;
const int ThreadCount = 4;

bool IsPrimeNumber(int number);
void PrintNumbers(const list<int>& primes);

int main() {
	int num = 1;
	recursive_mutex num_mutex, primes_mutex;
	list<int> primeNumbers;

	auto t0 = chrono::system_clock::now();
	list<shared_ptr<thread>> threads;

	for (int i = 0; i < ThreadCount; i++) {
		shared_ptr<thread> thread(
			new thread([&]() {
				// 각 스레드의 메인 함수 
				// 값을 가져올 수 있으면 루프를 돈다. 
				while (true) {
					int n;
					{
						lock_guard<recursive_mutex> num_lock(num_mutex);
						n = num;
						num++;
					}

					if (n >= MaxCount) break;
					if (IsPrimeNumber(n)) {
						lock_guard<recursive_mutex> primes_lock(primes_mutex);
						primeNumbers.push_back(n);
					}
				}
				}
			)
		); // 스레드 객체를 일단 갖고 있는다. 

		threads.push_back(thread);
	} // 모든 스레드가 일을 마칠 때까지 기다린다. 

	for (auto thread : threads) {
		thread->join();
	} // 끝 

	auto t1 = chrono::system_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();
	cout << "Took " << duration << " milliseconds." << endl;
	PrintNumbers(primeNumbers);
}

bool IsPrimeNumber(int number) {
	if (number == 1)return false;
	if (number == 2) return true;
	for (int i = 2; i < number; i++) {
		if (number % i == 0) return false;
	}
	return true;
}

void PrintNumbers(const list<int>& primes) {
	for (int i : primes) {
		cout << i << endl;
	}
}