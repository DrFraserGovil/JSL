#pragma once
#include <JSL/Strings/Serialisers.h>
namespace JSL::Vector
{
	// template<JSL::Concept::Numeric To, JSL::Concept::NonStringRange Origin>
	template<typename To, template<typename...> typename Container, typename From, typename... Args>
	requires (!std::convertible_to<To,std::string> && !std::convertible_to<From,std::string> && std::constructible_from<To,From>)
	Container<To> cast(const Container<From, Args...> & src)
	{
		return Container<To>(src.begin(), src.end());
	}

	
	template<typename To, template<typename...> typename Container, typename From, typename... Args>
	requires (std::convertible_to<To,std::string> && !std::convertible_to<From,std::string> )
	Container<To> cast(const Container<From, Args...> & src)
	{
		Container<To> result;
		std::transform(src.begin(),src.end(),std::inserter(result,result.end()), [](const From & x){
			return JSL::String::makeFrom(x);
		});
		return result;
	}

	template<typename To, template<typename...> typename Container, typename From, typename... Args>
	requires (std::convertible_to<From,std::string> )
	Container<To> cast(const Container<From, Args...> & src)
	{
		Container<To> result;
		std::transform(src.begin(),src.end(),std::inserter(result,result.end()), [](const From & x){
			return JSL::String::ParseTo<To>(x);
		});
		return result;
	}
}