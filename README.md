# SummerCoroutine
在阅读了vczh的[文章](https://zhuanlan.zhihu.com/p/25959684)后对[协程](https://en.wikipedia.org/wiki/Coroutine)(coroutine)的实现原理产生了一些兴趣，然而因为对其所提到的代码变换还不是太理解所以暂时只能通过较为简单的方式在C++中实现协程（三本我都考不上了...）。
<br/>
<br/>
基本上只是对Windows平台提供纤程做了一个简单的封装，上下文切换操作系统都帮忙干了...其他的平台也是类似，就不花时间了。
<br/>

# Example
```C++
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
```
效果如下：
![test](https://github.com/Locked-Cat/SummerCoroutine/blob/master/test.png)
<br/>
<br/>
大概稍稍了解了一点协程的实现原理，但是距离boost.coroutine或者libgo这些成熟的项目还有很长的距离。
