#pragma once
#include "Field.h"
#include <JSL/Strings/MakeFrom.h>
#include <map>
#include <string>
#include <typeinfo>
namespace JSL::Interface
{
	// forward declaration for the friend declaration
	namespace internal
	{
		class HelpGroup;
	} // namespace internal

	//! A set of strings used to configure the contents of the Help Message
	class HelpMetaData
	{
	  public:
		//! The string which immediately follows "./program" when the usage is declared.
		//! @details Set to empty to disable the usage section

		std::string UsageString = " cmd1 cmd2... -flag -key value cmd3\n";

		//! A string which appears (if ASCI active) in greyed out text beneath the UsageString declaration
		std::string UsageSubstring = "Keys are be indicated by at least one dash.\nFlags are booleans implicitly set to `true'.\nCommands are any space-delimited arguments not assigned to a key.\n";

		//! The 'title' of the program, entered in a summary section.
		std::string ProgramName = "Program Summary";

		//! The text which is entered into the summary section after the title
		//! @details Set to empty to disable the summary section
		std::string ProgramDescription = "";

		//! A list of commands or positional arguments accepted by the code, and a description of what they do
		std::map<std::string, std::string> Commands;

		friend class JSL::Interface::internal::HelpGroup;

	  private:
		//! The name of the program used to call this, extracted from argv[0] at runtime
		std::string CallingName = "";
	};
	namespace internal
	{
		//! Aggregates the help information from a given Aggegrator, and contains nested iterations for each nested Aggregate
		class HelpGroup
		{
		  public:
			/*!	Initialises the group
				@param name The Name field of the calling Aggregator
			*/
			HelpGroup(std::string name);

			/*!	Creates a child group, storing it within Nested
				@param name The Name field of the nested Aggregator
				@returns A reference to the newly created group
			*/
			HelpGroup &NestGroup(std::string name);

			/*! @brief Stringifies & formats a Field from a FieldList for display purposes
				@tparam The underlying type of the field
				@param field An object extracted from a FieldList in the Aggregator
			 */
			template <typename T>
			void AddField(Field<T> &field)
			{
				Format(field.Name, TypeString(typeid(T)), String::makeFrom(field.Default), field.Help, field.Aliases);
			}
			/*! @brief Register a list of commands for display
				@param cmds A map of commands and their documentation
			 */
			void AddCommands(std::map<std::string, std::string> &cmds);

			/*!	Output the contents of the
				@details If the metadata is non-empty, this is a signal that the Print was called by the Root Aggregator, triggering the additional display of the Usage and Summary fields
				@param meta An instance of the HelpData of the Aggregator, used to determine if Usage and Summary fields should be printed
			 */
			void Print(HelpMetaData meta = {});

		  private:
			//! The line-wrap width used for tabular formatting
			size_t lineWidth;

			//! The max character count in the left hand column
			//! If < 20, the column is shrunk to this size, so as not to waste space
			size_t MaxLWidth = 0;
			//! The max character count in the middle column
			//! If < 20, the column is shrunk to this size, so as not to waste space
			size_t MaxMWidth = 0;

			//! Tracks how deeply nested the current group is
			size_t Depth;

			//! The name assigned to this settings group
			std::string Name;

			//! A list of groups nested within the associated Aggregator
			std::vector<HelpGroup> Nested;

			//! A simple map of commands and a description to be printed by the Root Aggegrator
			//! @details Assigned by AddCommands
			std::vector<std::pair<std::string, std::string>> Commands = {};

			//! A list of the data captured from the fields, stored as left, middle and right hand columns
			std::vector<std::tuple<std::string, std::string, std::string>> Fields;

			/*! Formats the input Field object, and stores the result in the Fields member
				@param name The display name of the Field
				@param type A stringified representation of the typeid.name()
				@param defaultValue A stringified representation of the default value assigned to the field
				@param helpmsg The documentation assigned to this field
				@param aliases A list of aliases unique to the field
			 */
			void Format(std::string name, std::string type, std::string defaultValue, std::string helpmsg, std::vector<std::string> &aliases);

			/*! Converts typeid(T) into a non-garbled type representaion for common types
				@param info The type-info of a variable
				@returns A string representing the variable. For common inbuilt types, this is un-garbled.
			 */
			std::string TypeString(const std::type_info &info);
		};

	} // namespace internal
} // namespace JSL::Interface
