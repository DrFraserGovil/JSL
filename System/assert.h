#pragma once


#include "error.h"


namespace JSL
{
	//use internal namespace for internal workings of JSL which shouldn't be exposed to the user unless they *really* want them
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

	/*!
		A function which checks that a stated set of conditions are met, and throws a JSL::Error if they are not. Variadic function, and so accepts an arbitrary number of conditions without impact on runtime. \param assertionMessage The message that gets printed out if the assertion fails. Also useful for internal documentation of code \param args A list of booleans to be checked
	*/
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