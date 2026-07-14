#pragma once
#include <cstddef>
namespace JSL::Log
{
	//! TODO: This needs documenting fully
	class Indent
	{
	  public:
		Indent(size_t amount = 1);
		~Indent();

	  private:
		size_t Amount;
	};

} // namespace JSL::Log
