#include "include/coroutine.h"
#include <iostream>
#include <string>

using namespace summer;
using namespace std;

void foo(string name, int start)
{
	for (auto i = 0; i < 5; ++i)
	{
		cout << name << ": " << (start + i) << endl;
		yield;
	}
}

int main()
{
	Scheduler scheduler;
	auto _1 = scheduler.CreateCoroutine(foo, "1", 1);
	auto _2 = scheduler.CreateCoroutine(foo, "2", 2);
	while (_1.resume() && _2.resume());
	return 0;
}