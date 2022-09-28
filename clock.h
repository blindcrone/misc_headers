/* Some clock stuff, like wallclock benchmarks and framelocking
 * Made by blindcrone under WTFPL */
#pragma once
#include <chrono>
#include <memory>
#include <concepts>
#include "ringbuffer.h"
template <size_t W, std::floating_point Result = float>
struct moving_normal_dist {
	Result mu = 0
		 , var = 0
		 ;
	ring<Result, W> samples;
	void operator()(const auto v){
		const Result a = v
				   , f = samples.full() ? samples.front() : 0
				   , s = W
				   , d = a - f
				   , cm = mu
				   ;
		samples.push_back(a);
		mu += d / s;
		var += d * (a + f - mu - cm);
	}
};

template<size_t FPS, class Clock = std::chrono::high_resolution_clock>
static inline auto busywait_lock(const decltype(Clock::now())& start){
	std::chrono::duration<float,std::ratio<1,FPS>> elapsed;
	do elapsed = Clock::now() - start;
	while (elapsed.count() < 1);
	return elapsed.count();
}

template<size_t Unit = 1000, class Clock = std::chrono::steady_clock>
static inline auto wallclock(auto&& f, auto&&... a){
	typedef std::chrono::duration<float,std::ratio<1,Unit>> frame_t;
	auto start = Clock::now();
	if constexpr (std::same_as<std::invoke_result_t<decltype(f)&, decltype(a)&...>, void>)
		return f(a...), frame_t(Clock::now() - start).count();
	else
		return std::make_tuple(f(a...), frame_t(Clock::now() - start).count());
}

template <size_t Unit = 1000, size_t Freq = 60, typename... Args>
static inline auto time_average(std::invocable<Args...> auto&& f){
	typedef std::chrono::steady_clock Clock;
	auto st = std::make_shared<moving_normal_dist<Freq>>();
	const auto report = [=](const float rc){ return (*st)(rc), st->mu; };
	if constexpr (std::same_as<std::invoke_result_t<decltype(f)&, Args&...>, void>)
		return [=](Args&&... a){
			return report(wallclock<Unit, Clock>(f, a...));
		};
	else
		return [=](Args&&... a){
			auto [r, t] = wallclock<Unit, Clock>(f, a...);
			return std::make_tuple(r, report(t));
		};
}

template <size_t FPS = 60, typename... Args>
static inline auto frame_lock(std::invocable<Args...> auto&& f){
	typedef std::chrono::steady_clock Clock;
	if constexpr (std::same_as<std::invoke_result_t<decltype(f)&, Args&...>, void>)
		return [=](Args&&... a){
			auto start = Clock::now();
			f(a...), busywait_lock<FPS, Clock>(start);
		};
	else
		return [=](Args&&... a){
			auto start = Clock::now();
			auto ret = f(a...);
			return busywait_lock<FPS, Clock>(start), ret;
		};
}

