#pragma once
#include <sstream>
#include <type_traits>
#include <functional>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <array>

namespace MetaTraits
{
	template<typename... T>
	struct m_void
	{
		using type = void;
	};

	template<typename... T>
	using void_t = typename m_void<T...>::type;

	template<typename T>
	struct identity
	{
		using type = T;
	};

	template<typename T>
	using identity_t = typename identity<T>::type;

	template<typename T>
	struct remove_cv_ref : std::remove_reference<typename std::remove_cv<T>::type> {};

	template<typename T>
	using remove_cv_ref_t = typename remove_cv_ref<T>::type;

	template<typename Ins, template<typename...> class Tmp>
	struct is_instance :std::false_type {};

	template<template<typename...> class Tmp, typename... Args>
	struct is_instance<Tmp<Args...>, Tmp> :std::true_type {};

	template<typename...>
	struct get_lastType;

	template<typename T1, typename T2, typename...Args>
	struct get_lastType<T1, T2, Args...> :get_lastType<T2, Args...> {};

	template<typename T>
	struct get_lastType<T> : identity<T> {};

	//将 integral_constant 元函数 柯里化
	template<bool Val>
	using my_Cat_Base = std::integral_constant<bool, Val>;

	template<typename T>
	struct is_stringstream : std::is_same<T, std::stringstream> {};

	template<typename T, typename = void>
	struct is_iterable_container : std::false_type {};

	template<typename T>
	struct is_iterable_container <T, void_t<typename T::iterator, typename T::const_iterator>> : std::true_type {};

	template<typename T>
	struct is_vector : is_instance<T, std::vector> {};

	template<typename T>
	struct is_deque : is_instance<T, std::deque> {};

	template<typename T>
	struct is_list : is_instance<T, std::list> {};

	template<typename T>
	struct is_set : is_instance<T, std::set> {};

	template<typename T>
	struct is_unordered_set : is_instance<T, std::unordered_set> {};

	template<typename T>
	struct is_map : is_instance<T, std::map> {};

	template<typename T>
	struct is_unordered_map : is_instance<T, std::unordered_map> {};

	template<typename T>
	struct is_stack : is_instance<T, std::stack> {};

	template<typename T>
	struct is_queue : is_instance<T, std::queue> {};

	template<typename T>
	struct is_priority_queue : is_instance<T, std::priority_queue> {};

	template<typename T>
	struct is_container : my_Cat_Base<is_vector<T>::value || is_deque<T>::value || is_list<T>::value ||
		is_set<T>::value || is_unordered_set<T>::value || is_map<T>::value || is_unordered_map<T>::value ||
		is_stack<T>::value || is_queue<T>::value || is_priority_queue<T>::value> {};

	template<typename T>
	struct is_shared_ptr : is_instance<T, std::shared_ptr> {};

	template<typename T>
	struct is_const_char_pointer : std::is_same<typename std::decay<T>::type, char const*> {};

	template<typename T>
	struct is_string : std::is_same<T, std::string> {};

	template<typename T, typename U, typename... Rest>
	struct is_one_of :std::conditional<is_one_of<T, U>::value, std::true_type, is_one_of<T, Rest...>>::type {};

	template<typename T, typename U>
	struct is_one_of<T, U> :std::conditional<std::is_same<T, U>::value, std::true_type, std::false_type>::type {};

	template<typename Fn, typename... Args,
		typename std::enable_if<std::is_member_pointer<typename std::decay<Fn>::type>::value, int>::type = 0>
		constexpr auto invoke(Fn&& f, Args&&... args)
		noexcept(noexcept(std::mem_fn(f)(std::forward<Args>(args)...)))
		-> decltype(std::mem_fn(f)(std::forward<Args>(args)...))
	{
		return std::mem_fn(f)(std::forward<Args>(args)...);
	}

	template<typename Fn, typename... Args,
		typename std::enable_if<!std::is_member_pointer<typename std::decay<Fn>::type>::value, int>::type = 0>
		constexpr auto invoke(Fn&& f, Args&&... args)
		noexcept(noexcept(std::forward<Fn>(f)(std::forward<Args>(args)...)))
		->decltype(std::forward<Fn>(f)(std::forward<Args>(args)...))
	{
		return std::forward<Fn>(f)(std::forward<Args>(args)...);
	}

	//重载<<运算符，便于std::array输出调试
	template<typename Elem, typename Traits, typename ValueType, std::size_t size> inline
		std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& oStream, std::array<ValueType, size> const& ary)
	{
		if (!ary.empty())
		{
			for (typename std::remove_reference<decltype(ary)>::type::size_type i = 0; i < ary.size() - 1; ++i)
			{
				oStream << ary[i] << ", ";
			}
			oStream << ary[ary.size() - 1];
		}

		return oStream;
	}

	//重载<<运算符，便于std::vector输出调试
	template<typename Elem, typename Traits, typename ValueType, typename Alloc> inline
		std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& oStream, std::vector<ValueType, Alloc> const& vec)
	{
		using vector_type = typename std::remove_reference<decltype(vec)>::type;

		if (!vec.empty())
		{
			typename vector_type::const_iterator end = --vec.cend();
			for (typename vector_type::const_iterator iter = vec.cbegin(); iter != end; ++iter)
			{
				oStream << *iter << ", ";
			}
			oStream << *vec.crbegin();
		}

		return oStream;
	}

	//获取 强枚举类型 的 基类型 对应值
	template<typename T> inline
		constexpr typename std::underlying_type<T>::type enumToCode(T a)
	{
		return static_cast<typename std::underlying_type<T>::type>(a);
	}

	//获取 基类型 的 强枚举类型 对应值
	template<typename T> inline
		constexpr T codeToEnum(typename std::underlying_type<T>::type a)
	{
		return static_cast<T>(a);
	}

	//单例模板(线程安全)
	template<typename T>
	class Singleton
	{
	private:
		Singleton() = default;

	public:
		using Myt = Singleton<T>;

		Singleton(Myt const&) = delete;

		Singleton& operator=(Myt const&) = delete;

		~Singleton() = default;

		static T* getInstance(void)
		{
			static T val;
			return &val;
		}
	};

	//单例模板，线程不安全，构造方式多元灵活，有释放资源的接口,使用 getInstance 前需要先调用 initialize 初始化
	//Notice: initialize 时，若之前已持有资源则会抛出异常，可使用 isRelease 判断单例中资源是否释放
	template<typename T>
	class SingletonThreadUnsafe
	{
	private:
		SingletonThreadUnsafe() = default;

	public:
		using Myt = SingletonThreadUnsafe<T>;

		SingletonThreadUnsafe(Myt const&) = delete;

		SingletonThreadUnsafe& operator=(Myt const&) = delete;

		~SingletonThreadUnsafe() = default;

		static bool isRelease()
		{
			return *getPPoint() == nullptr ? true : false;
		}

		static void release()
		{
			if (*getPPoint() != nullptr)
			{
				delete* getPPoint();
				*getPPoint() = nullptr;
			}
		}

		template<typename... Args>
		static T* initialize(Args&&... args)
		{
			if (*getPPoint() == nullptr)
			{
				*getPPoint() = new T(std::forward<Args>(args)...);
			}
			else
			{
				throw "The singleton has been initialized";
			}

			return *getPPoint();
		}

		static T* getInstance()
		{
			return *getPPoint();
		}

	private:
		static T** getPPoint()
		{
			static T* _valPoint = nullptr;
			return &_valPoint;
		}
	};

}

