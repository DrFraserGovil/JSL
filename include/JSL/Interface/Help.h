#pragma once
#include "Field.h"
#include <JSL/Strings/MakeFrom.h>
#include <map>
#include <string>
#include <typeinfo>
namespace JSL::Interface
{
	struct HelpMetaData
	{
		std::string CallingName = "";
		std::string UsageString = " cmd1 cmd2... -flag -key value cmd3\n";
		std::string UsageSubstring = "Keys are be indicated by at least one dash.\nFlags are booleans implicitly set to `true'.\nCommands are any space-delimited arguments not assigned to a key.\n";
		std::string ProgramName = "";
		std::string ProgramDescription = "";
		std::map<std::string, std::string> Commands;
	};
	namespace internal
	{
		class HelpGroup
		{
		  public:
			HelpGroup(std::string name);

			HelpGroup &NestGroup(std::string name);
			template <typename T>
			void AddField(Field<T> &field)
			{
				Format(field.Name, TypeString(typeid(T)), String::makeFrom(field.Default), field.Help, field.Aliases);
			}
			void AddCommands(std::map<std::string, std::string> &cmds);
			void Print(HelpMetaData meta = {});

		  private:
			size_t lineWidth = 80;
			size_t MaxLWidth = 0;
			size_t MaxMWidth = 0;
			size_t Depth;
			std::string Name;
			std::vector<HelpGroup> Nested;
			std::vector<std::pair<std::string, std::string>> Commands = {};
			std::vector<std::tuple<std::string, std::string, std::string>> Fields;
			void Format(std::string name, std::string type, std::string defaultValue, std::string helpmsg, std::vector<std::string> &aliases);
			std::string TypeString(const std::type_info &info);
		};

	} // namespace internal
} // namespace JSL::Interface
