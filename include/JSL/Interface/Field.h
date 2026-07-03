#pragma once
#include <string>
#include <vector>
namespace JSL::Interface
{
	template <typename T>
	struct Field
	{
		T &Value; // reference to the real member
		std::string Name;
		std::vector<std::string> Aliases; // metadata only
		T Default;
		std::string Help;
		Field(T &value, std::string name, std::vector<std::string> aliases, T def, std::string help) : Value(value), Name(name), Aliases(aliases), Default(def), Help(help)
		{
		}
	};
	template <typename T>
	concept IsField = requires(T t) {
		t.Name;
		t.Value;
		t.Default;
		t.Aliases;
		t.Help;
	};

#define JSL_Field(variable, aliases, default, documentation) \
	Field { variable, #variable, aliases, default, documentation }

} // namespace JSL::Interface
