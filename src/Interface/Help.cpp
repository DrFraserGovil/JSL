#include <JSL/Display/Format.h>
#include <JSL/Display/Terminal.h>
#include <JSL/Interface/Help.h>
#include <JSL/Strings/Wrap.h>
#include <iostream>
#include <map>
#include <regex>
#define insert(type) {typeid(type).name(), std::regex_replace(#type, std::regex("std::"), "")}
inline const std::map<std::string, std::string> CommonTypes = {
	insert(int),
	insert(double),
	insert(std::string),
	insert(bool),
	insert(char),
	insert(std::vector<int>),
	insert(std::vector<std::string>),
	insert(std::vector<double>),
	insert(size_t)};
#undef insert
namespace JSL::Interface::internal
{

	std::string HelpGroup::TypeString(const std::type_info &t)
	{
		std::string tname = t.name();
		auto it = CommonTypes.find(tname);
		return (it != CommonTypes.end()) ? CommonTypes.at(tname) : tname;
	}

	HelpGroup::HelpGroup(std::string name) : Name(name)
	{
		Depth = 0;
	}
	HelpGroup &HelpGroup::NestGroup(std::string name)
	{
		Nested.emplace_back(name);
		auto &out = Nested.back();
		out.Depth = Depth + 1;
		return out;
	}

	void HelpGroup::Format(std::string name, std::string type, std::string defaultValue, std::string helpmsg, std::vector<std::string> &aliases)
	{
		auto left = "-" + aliases[0];
		for (size_t i = 1; i < aliases.size(); ++i)
		{
			left += ", -" + aliases[i];
		}
		auto mid_text = "<" + std::string(type) + ">";
		mid_text = Display::Colour(0, 155, 185) + std::string(name) + Display::ResetAll() + "\n" + std::regex_replace(mid_text, std::regex("std::"), "");

		std::string right_text(helpmsg);
		right_text += "\n" + (std::string)Display::Italics() + Display::Colour(50, 50, 50) + "[Default: " + (std::string)defaultValue + "]" + Display::ResetAll();

		// + Display::Italics + "\n[Default: " + DefaultValue + "]";
		size_t lw = 20;
		size_t mw = 20;
		size_t rw = lineWidth - lw - mw;
		Fields.push_back(String::tableFormat({left, mid_text, right_text}, {lw, mw, rw}, " "));
	}

	void HelpGroup::AddCommands(std::map<std::string, std::string> &cmds)
	{

		for (auto &[cmd, doc] : cmds)
		{
			Commands += String::tableFormat({cmd, doc}, {20, lineWidth - 20});
		}
	}

	void TitlePrint(std::string_view title, size_t lineWidth)
	{
		auto fg = Display::Black();
		auto bg = Display::White(true);

		if (Display::Terminal().IsANSICapable()) // piggyback off the terminal checking if formatting can be used
		{
			auto titleCol = fg + Display::Italics() + bg;
			std::string buffer = (title.size() < lineWidth) ? std::string(lineWidth - title.size(), ' ') : "";
			std::cout << titleCol << title << buffer << Display::ResetAll() << "\n";
		}
		else
		{
			size_t amount = std::max(1, (int)(0.5 * (lineWidth - 2 - title.size())));
			std::string tag(amount, '=');
			std::cout << "\n"
					  << tag << " " << title << " " << tag << "\n\n";
		}
	}

	void HelpGroup::Print(std::string executableName)
	{
		if (!executableName.empty())
		{

			std::cout << "Usage:\n";
			std::cout << "\t" << executableName << " cmd1 cmd2... -flag -key value cmd3\n";
			std::cout << Display::Colour(40, 40, 40) << Display::Italics();
			std::cout << "Keys are be indicated by at least one dash.\nFlags are booleans implicitly set to `true'.\nCommands are any space-delimited arguments not assigned to a key.\n";
			if (!Commands.empty())
			{
				TitlePrint("Commands", lineWidth);
				std::cout << Commands;
			}
		}
		TitlePrint(Name, lineWidth);
		std::sort(Fields.begin(), Fields.end());
		for (auto f : Fields)
		{
			std::cout << f << "\n";
		}
		std::sort(Nested.begin(), Nested.end(), [](auto &lhs, auto &rhs) { return lhs.Name < rhs.Name; });
		for (auto n : Nested)
		{
			n.Print();
		}
	}
} // namespace JSL::Interface::internal
