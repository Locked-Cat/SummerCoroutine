#pragma once

#include <Windows.h>
#include <map>
#include <functional>
#include <utility>

namespace summer
{
	class CoroutineBase;
	class CoroutineHandler;

	class Scheduler
	{
		template <typename Func, typename ...Args>
		friend class Coroutine;
	public:
		Scheduler();

		Scheduler(const Scheduler&) = delete;
		Scheduler& operator=(const Scheduler&) = delete;

		template <typename Func, typename ...Args>
		CoroutineHandler CreateCoroutine(Func&& func, Args&&... args);
		void Resume(CoroutineHandler coroutineHandler);
		static void YieldToMain();
	private:
		static void WINAPI CallWrapper(::LPVOID parameters);

		::LPVOID _main;
		std::map<CoroutineBase*, ::PVOID> _coroutines;
		static std::map<::PVOID, Scheduler*> fibers;
	};

	template<typename Func, typename ...Args>
	CoroutineHandler Scheduler::CreateCoroutine(Func&& func, Args&& ...args)
	{
		auto newCoroutine = static_cast<CoroutineBase*>(new Coroutine<Func, Args...>{ *this, std::forward<Func>(func), std::forward<Args>(args)... });
		_coroutines.insert(std::make_pair(newCoroutine, static_cast<::PVOID>(NULL)));
		return CoroutineHandler{ newCoroutine };
	}
}

#define yield ::summer::Scheduler::YieldToMain()