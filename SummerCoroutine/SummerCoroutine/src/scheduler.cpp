#include "../include/scheduler.h"
#include "../include/coroutine.h"
#include <cassert>
#include <algorithm>
#include <iostream>

using namespace summer;

std::map<::PVOID, Scheduler*> Scheduler::fibers;

Scheduler::Scheduler()
	: _main(::ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH))
{
}

void Scheduler::Resume(CoroutineHandler coroutineHandler)
{
	auto coroutine = coroutineHandler._coroutine;
	switch (coroutine->State())
	{
	case CoroutineBase::CoroutineState::READY:
		_coroutines[coroutine] = ::CreateFiberEx(0, 0, FIBER_FLAG_FLOAT_SWITCH, &Scheduler::CallWrapper, static_cast<::LPVOID>(coroutine));
		assert(_coroutines[coroutine] != NULL);
		fibers[_coroutines[coroutine]] = this;
		break;
	case CoroutineBase::CoroutineState::SUSPEND:
		assert(_coroutines[coroutine] != NULL);
		break;
	default:
		assert(false);
	}
	coroutine->State() = CoroutineBase::CoroutineState::RUNNING;
	::SwitchToFiber(_coroutines[coroutine]);

	if (coroutine->State() == CoroutineBase::CoroutineState::DEAD)
	{
		::DeleteFiber(_coroutines[coroutine]);
		_coroutines.erase(coroutine);
	}
}

void Scheduler::YieldToMain()
{
	auto currentFiber = ::GetCurrentFiber();
	assert(fibers.find(currentFiber) != fibers.end());
	auto ownScheduler = fibers[currentFiber];

	auto currentCoroutineIterator = std::find_if(ownScheduler->_coroutines.begin(), ownScheduler->_coroutines.end(),
		[currentFiber](std::pair<CoroutineHandler, ::PVOID> value) {return value.second == currentFiber; });
	assert(currentCoroutineIterator != ownScheduler->_coroutines.end());
	auto currentCoroutine = currentCoroutineIterator->first;
	currentCoroutine->YieldToMain();
}

void Scheduler::CallWrapper(::LPVOID parameters)
{
	auto coroutine = static_cast<CoroutineBase*>(parameters);
	coroutine->Call();

	coroutine->State() = CoroutineBase::CoroutineState::DEAD;
	auto currentFiber = ::GetCurrentFiber();
	assert(fibers.find(currentFiber) != fibers.end());
	auto ownScheduler = fibers[currentFiber];

	::SwitchToFiber(ownScheduler->_main);
}
