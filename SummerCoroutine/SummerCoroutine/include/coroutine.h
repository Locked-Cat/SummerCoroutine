#pragma once

#include "scheduler.h"
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace summer
{
	class CoroutineBase
	{
	public:
		enum class CoroutineState
		{
			READY,
			SUSPEND,
			RUNNING,
			DEAD
		};

		CoroutineState& State()
		{
			return _state;
		}

		virtual void Call() = 0;
		virtual void Resume() = 0;
		virtual void YieldToMain() = 0;
		virtual ~CoroutineBase() {}
	private:
		CoroutineState _state = CoroutineState::READY;
	};

	class CoroutineHandler
	{
		friend class Scheduler;
	public:
		CoroutineHandler(CoroutineBase* coroutine)
			: _coroutine(coroutine) {}

		bool resume()
		{
			_coroutine->Resume();
			if (_coroutine->State() == CoroutineBase::CoroutineState::DEAD)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
	private:
		CoroutineBase* _coroutine;
	};

	template <typename Func, typename ...Args>
	class Coroutine
		: public CoroutineBase
	{
	public:
		Coroutine(summer::Scheduler& scheduler, Func&& coroutineFunc, Args&&... parameters);

		Coroutine(const Coroutine&) = delete;
		Coroutine& operator=(const Coroutine&) = delete;

		void Call() override;
		void YieldToMain() override;
		void Resume() override;
	private:
		template<typename F, typename Tuple, std::size_t ...I>
		void CallImpl(F&& func, Tuple&& tuple, std::index_sequence<I...>);

		summer::Scheduler& _scheduler;
		Func _coroutineFunc;
		std::tuple<Args...> _parameters;
	};

	template <typename Func, typename ...Args>
	Coroutine<Func, Args...>::Coroutine(summer::Scheduler& scheduler, Func&& coroutineFunc, Args&&... parameters)
		: _scheduler(scheduler)
		, _coroutineFunc(std::forward<Func>(coroutineFunc))
		, _parameters(std::tuple<Args...>(std::forward<Args>(parameters)...))
	{
	}

	template<typename Func, typename ...Args>
	template<typename F, typename Tuple, std::size_t... I>
	void Coroutine<Func, Args...>::CallImpl(F&& func, Tuple&& tuple, std::index_sequence<I...>)
	{
		std::invoke(std::forward<F>(func), std::get<I>(std::forward<Tuple>(tuple))...);
	}

	template<typename Func, typename ...Args>
	void Coroutine<Func, Args...>::Call()
	{
		CallImpl(_coroutineFunc, _parameters, std::make_index_sequence<sizeof...(Args)>{});
	}

	template<typename Func, typename ...Args>
	void Coroutine<Func, Args...>::YieldToMain()
	{
		State() = CoroutineState::SUSPEND;
		::SwitchToFiber(_scheduler._main);
	}

	template<typename Func, typename ...Args>
	void Coroutine<Func, Args...>::Resume()
	{
		auto coroutineHandler = static_cast<CoroutineBase*>(this);
		_scheduler.Resume(coroutineHandler);
	}
}