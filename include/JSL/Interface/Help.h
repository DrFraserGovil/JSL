#pragma once
#include "Field.h"
#include <JSL/Strings/MakeFrom.h>
#include <map>
#include <string>
#include <typeinfo>
namespace JSL::Interface::internal
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
		void Print(std::string executableName = "");

	  private:
		size_t lineWidth = 80;
		size_t Depth;
		std::string Name;
		std::vector<HelpGroup> Nested;
		std::string Commands = "";
		std::vector<std::string> Fields;
		void Format(std::string name, std::string type, std::string defaultValue, std::string helpmsg, std::vector<std::string> &aliases);
		std::string TypeString(const std::type_info &info);
	};

} // namespace JSL::Interface::internal
