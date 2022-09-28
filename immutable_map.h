/* Immutable Map Utils. Just some minor stuff for working with immutable maps */
/* Hacked together by blindcrone, under the WTFPL, lol */
#pragma once
#include <vector>
#include <algorithm>
#include <ranges>
#include <concepts>

template <typename F, typename... Args>
using irt = std::invoke_result_t<F, Args...>;

template <std::ranges::range T>
using rvt = std::ranges::range_value_t<T>;

template <typename K, typename V>
using immutable_map = std::vector<std::pair<K,V>>;

template <typename T>
concept numeric = std::integral<T> || std::floating_point<T>;

template <typename T>
concept initialized = std::default_initializable<T> && !numeric<T>;

template <numeric T>
constexpr T defaultval(){ return 0; }

template <initialized T>
constexpr T defaultval(){ return {}; }

template <typename K, typename V>
constexpr auto finde(const immutable_map<K,V>& m, const std::convertible_to<K> auto& k){
	typedef std::pair<K,V> item;
	return std::find_if(m.begin(), m.end(), [v = K(k)](const item& p){ return p.first == v; });
}

template <typename K, typename V>
constexpr V gette(const immutable_map<K,V>& m, const std::convertible_to<K> auto& k){
	auto i = finde(m, k);
	return i == m.end() ? defaultval<V>() : (*i).second;
}

template <typename K, typename V>
constexpr auto do_if_in( const immutable_map<K,V>& m, const std::convertible_to<K> auto& k
					   , const std::invocable<V> auto& f){
	if constexpr (std::same_as<irt<decltype(f), V>, void>){
		if (auto i = finde(m, k); i != m.end())
			f((*i).second);
	} else {
		if (auto i = finde(m, k); i != m.end())
			return f((*i).second);
		else
			return defaultval<irt<decltype(f), V>>();
	}
}

constexpr auto spread(const auto& op, const std::ranges::range auto& r){
	std::vector<irt<decltype(op), rvt<decltype(r)>>> v(r.size());
	return std::transform(r.begin(), r.end(), v.begin(), op), v;
}

constexpr auto index_on(const std::ranges::range auto& r, const std::ranges::range auto& i)
requires std::integral<rvt<decltype(i)>> {
	return spread([&](const auto n){ return r[n]; }, i);
}

