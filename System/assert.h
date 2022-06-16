#pragma once


#include "error.h"


namespace JSL
{
	namespace internal
	{
		template<typename... Ts>
		bool variadicAssertion(bool first, Ts... args)
		{
			if constexpr (sizeof...(args) > 0)
			{
				return first & variadicAssertion(args...);
			}
			else
			{
				return first;
			}
		}
	}
	template<typename... Ts>
	void inline Assert(std::string assertionMessage, Ts... args)
	{
		bool passing = internal::variadicAssertion(args...);

		if (!passing)
		{
			Error(FailedAssertion,assertionMessage);
		}
	};
}