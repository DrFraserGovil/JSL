#include <JSL/Log/Config.h>
#include <JSL/Log/Indent.h>
namespace JSL::Log
{
	Indent::Indent(size_t amount) : Amount(amount)
	{
		JSL::Log::Global().IndentLevel += amount;
	}
	Indent::~Indent()
	{
		int clevel = JSL::Log::Global().IndentLevel;
		if (clevel - (int)Amount < 0)
		{
			JSL::Log::Global().IndentLevel = 0;
		}
		else
		{
			JSL::Log::Global().IndentLevel -= Amount;
		}
	}

} // namespace JSL::Log
