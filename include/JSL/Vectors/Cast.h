#pragma once
#include <string>
#include <concepts>
#include <JSL/Strings/Serialisers.h>
namespace JSL::Vector
{
	/*!
	 * @brief A generalisation of static_cast to STL containers; generates a new object which contains a casting of the original internal type
	 @warning We make no guarantees about the reversibility of this process, or that the order of the objects are preserved (i.e. an std::set may sort differently in the To-space than the From-space).
	 * @tparam To The destination type held within the output container. Must be implicitly constructible from the origin type. Must not be a string-like type.
	 * @tparam From The origin type held within the original container. Must not be a string-like type.
	 * @tparam ...Args A catcher for other types (i.e. iterators) that are often hidden away by STL containers
	 * @param src The original container to be cast into a new type.
	 * @return A new container containing the same objects as the original, but constructed into a new type.
	 */
	template<typename To, template<typename...> typename Container, typename From, typename... Args>
	requires (!std::convertible_to<To,std::string> && !std::convertible_to<From,std::string> && std::constructible_from<To,From>)
	Container<To> cast(const Container<From, Args...> & src)
	{
		return Container<To>(src.begin(), src.end());
	}

	/*!
	 * @brief A generalisation of the JSL::String::makeFrom routine, applied to STL containers. Generates a new container holding a string representation of the individual elements of the original container.
	 @warning We make no guarantees about the reversibility of this process, or that the order of the objects are preserved (i.e. an std::set may sort differently in the To-space than the From-space).
	 * @tparam To A string-like type
	 * @tparam From The origin type to be converted into a string.
	 * @tparam ...Args A catcher for other types (i.e. iterators) that are often hidden away by STL containers
	 * @param src The original container to be cast into a string-type.
	 * @return A new container containing the same objects as the original, but represented as strings.
	 */
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

	/*!
	 * @brief A generalisation of the JSL::String::ParseTo routine, applied to STL containers. Generates a new container holding a parsed version of the strings in the input.
	 @warning We make no guarantees about the reversibility of this process, or that the order of the objects are preserved (i.e. an std::set may sort differently in the To-space than the From-space).
	 * @tparam To A type supported by the JSL::String::ParseTo templates.
	 * @tparam From A string like type which can be parsed.
	 * @tparam ...Args A catcher for other types (i.e. iterators) that are often hidden away by STL containers
	 * @param src The original container of strings which are to be parsed.
	 * @return A new container containing the same objects as the original, but parsed into the target type.
	 */
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
