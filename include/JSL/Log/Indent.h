#pragma once
#include <cstddef>
namespace JSL::Log
{
	/// @brief An object that, whilst in scope, increases the overall indent level of input passed to the Logger. Useful for ensuring entire blocks of log are indented, without relying on chaining "\t\t\t" calls
	/// @details Upon construction, increments the global Config::Indent level, and decrements it upon completion.
	class Indent
	{
	  public:
		/// @brief Constructs the indent-holder and increments the global counter
		/// @param amount The number of indents (relative to the current level) to increase the indent level by. Each indent has a width determined by the Config
		Indent(size_t amount = 1);

		/// @brief The destructor; decrements the indent level back by the original amount
		~Indent();

	  private:
		/// @brief Store the assigned indent level, so it can be undone when the object passes out of scope
		size_t Amount;
	};

} // namespace JSL::Log
