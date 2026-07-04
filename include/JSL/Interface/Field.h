#pragma once
#include <string>
#include <vector>
namespace JSL::Interface
{

	//! Used to declare entries in an Aggregator object's FieldList member; connected the input value to the metadata about it
	template <typename T>
	class Field
	{
	  public:
		//! A reference to the external value connected to this field.
		//! This value should be a non-const member variable of an Aggregator child class
		T &Value;

		//! The display name of the field. Usually a stringified version of the parameter passed to Value.
		//! @details This value is for display purposes only; it is not used for identifiying the field within the object itself
		std::string Name;

		//! A list of aliases, to be assigned to this field's Context
		std::vector<std::string> Aliases; // metadata only

		//! The default value to be assigned to the Value if the Parser did not recieve an override
		T Default;

		//! The message to be displayed when Help mode is activated; documentation as to the purpose and role of this field within the code
		std::string Help;

		//! Basic constructor.
		//! @tparam T The Context object infers the KeyType value based on this type.
		//! @param value The object to be bound-by-reference to the Value stored in the field; the Field is then bound to this object
		//! @param name The display name of the field
		//! @param aliases The list of aliases to be assigned to this field's Context
		//! @param def The default value to be assigned to the value field if no override is parsed
		//! @param help Help message documentation, detailing the purpose of this field
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

/*! @brief Constructs a JSL::Interface::Field object where the name field is populated automatically at compile time by the stringified variable name
	@details This macro may misbehave when applied to string variables, due to the myriad types associated with them not enabling type inference. When using strings, it is best to manually define the Field without this macro.
*/
#define JSL_Field(variable, aliases, default, documentation) \
	Field { variable, #variable, aliases, default, documentation }

} // namespace JSL::Interface
