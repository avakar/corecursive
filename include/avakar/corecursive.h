#ifndef __has_include
# error corecursive: you need a compiler with support for C++ coroutines
#endif

#if __has_include(<coroutine>)
#include <coroutine>
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>

namespace std {

using std::experimental::coroutine_handle;
using std::experimental::suspend_always;
using std::experimental::suspend_never;

}
#else
# error corecursive: you need a compiler with support for C++ coroutines
#endif

#include <cassert>
#include <exception>
#include <type_traits>
#include <utility>

namespace avakar {
namespace _corecursive {

template <typename T>
struct corecursive;

struct _promise_base
{
	virtual void resume() noexcept = 0;

	bool done() const noexcept
	{
		return blocker == nullptr;
	}

	_promise_base * parent = nullptr;
	_promise_base * blocker = this;
	std::exception_ptr exc;
};

template <typename T, typename = void>
struct _promise_value_mixin
	: _promise_base
{
	_promise_value_mixin()
	{
	}

	void return_value(T v)
	{
		new(&_storage) T(std::move(v));
		this->blocker = nullptr;
	}

	T & value()
	{
		return *reinterpret_cast<T *>(&_storage);
	}

	T move_value()
	{
		return std::move(this->value());
	}

	~_promise_value_mixin()
	{
		if (blocker == nullptr && !exc)
			this->value().~T();
	}

	_promise_value_mixin(_promise_value_mixin const &) = delete;
	_promise_value_mixin & operator=(_promise_value_mixin const &) = delete;

private:
	alignas(T) char _storage[sizeof(T)];
};

template <typename T>
struct _promise_value_mixin<T, std::enable_if_t<std::is_void_v<T>>>
	: _promise_base
{
	void return_void()
	{
		this->blocker = nullptr;
	}

	void move_value()
	{
	}

	~_promise_value_mixin()
	{
	}
};


template <typename T>
struct corecursive
{
	struct promise_type final
		: _promise_value_mixin<T>
	{
		using value_type = T;

		void resume() noexcept override
		{
			std::coroutine_handle<promise_type>::from_promise(*this).resume();
		}

		corecursive<T> get_return_object() noexcept
		{
			return corecursive<T>(*this);
		}

		void unhandled_exception() noexcept
		{
			this->exc = std::current_exception();
			this->blocker = nullptr;
		}

		auto initial_suspend() const noexcept
		{
			return std::experimental::suspend_always{};
		}

		auto final_suspend() const noexcept
		{
			return std::experimental::suspend_always{};
		}
	};

	corecursive(corecursive && o) noexcept
		: _promise(o._promise)
	{
		o._promise = nullptr;
	}

	corecursive & operator=(corecursive && o) noexcept
	{
		std::swap(_promise, o._promise);
		return *this;
	}

	~corecursive() noexcept(false)
	{
		if (_promise)
		{
			_promise_deleter deleter{ _promise };
			if (_promise->blocker)
				this->get();
		}
	}

	bool await_ready() const noexcept
	{
		return _promise->done();
	}

	template <typename P>
	auto await_suspend(std::coroutine_handle<P> h) noexcept
		-> std::enable_if_t<std::is_same_v<P, typename corecursive<typename P::value_type>::promise_type>>
	{
		auto & parent_promise = h.promise();
		parent_promise.blocker = _promise;
		_promise->parent = &parent_promise;
	}

	T await_resume()
	{
		assert(_promise != nullptr);
		assert(_promise->blocker == nullptr);
		if (_promise->exc)
			std::rethrow_exception(_promise->exc);
		return _promise->move_value();
	}

	T get()
	{
		assert(_promise != nullptr);
		this->_resolve();
		return this->await_resume();
	}

	operator T()
	{
		return this->get();
	}

private:
	void _resolve()
	{
		_promise_base * cur = _promise;
		while (cur)
		{
			if (cur->done())
			{
				cur = cur->parent;
				if (cur)
					cur->resume();
			}
			else
			{
				_promise_base * next = cur->blocker;
				if (next)
					cur = next;
				cur->resume();
			}
		}
	}

	struct _promise_deleter
	{
		~_promise_deleter()
		{
			std::coroutine_handle<promise_type>::from_promise(*promise).destroy();
		}

		promise_type * promise;
	};

	explicit corecursive(promise_type & promise)
		: _promise(&promise)
	{
	}

	promise_type * _promise;
};

}

template <typename T>
using corecursive = _corecursive::corecursive<T>;

}

#pragma once
