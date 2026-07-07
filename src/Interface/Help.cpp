#include <JSL/Display/Format.h>
#include <JSL/Display/Terminal.h>
#include <JSL/Interface/Help.h>
#include <JSL/Log.h>
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

	HelpGroup::HelpGroup(std::string name, size_t *lwidth, size_t *mwidth) : Name(name), MaxLWidth(lwidth), MaxMWidth(mwidth)
	{
		Depth = 0;
	}
	HelpGroup &HelpGroup::NestGroup(std::string name)
	{
		Nested.emplace_back(name, MaxLWidth, MaxMWidth);
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
		auto mid_text = Display::Colour(80, 155, 185) + std::string(name) + Display::ResetAll() + "\n<" + type + ">";

		std::string right_text(helpmsg);
		right_text += "\n" + (std::string)Display::Italics() + Display::Colour(50, 50, 50) + "[Default: " + (std::string)defaultValue + "]" + Display::ResetAll();

		Fields.push_back({left, mid_text, right_text});
		size_t ms = std::max(2 + type.size(), name.size());
		size_t ls = String::trueSize(left);
		*MaxMWidth = std::max(ms, *MaxMWidth);
		*MaxLWidth = std::max(ls, *MaxLWidth);
	}

	void HelpGroup::AddCommands(std::map<std::string, std::string> &cmds)
	{

		for (auto &[cmd, doc] : cmds)
		{
			Commands.push_back({cmd, doc});
			*MaxLWidth = std::max(*MaxLWidth, cmd.size());
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

	void HelpGroup::Print(HelpMetaData meta)
	{

		int width = std::min(120ul, JSL::Display::Terminal().Columns());
		size_t minbuff = 3;
		size_t l = std::min(20ul, *MaxLWidth + minbuff);
		size_t m = std::min(20ul, *MaxMWidth + minbuff);
		size_t remainder = std::max(20, (int)(width - l - m));
		lineWidth = remainder + l + m;
		if (!meta.CallingName.empty())
		{

			std::cout << "Usage:\n";
			std::cout << "\t" << meta.CallingName << meta.UsageString;
			std::cout << Display::Colour(40, 40, 40) << Display::Italics();
			std::cout << meta.UsageSubstring;
			if (!Commands.empty())
			{
				TitlePrint("Commands", lineWidth);
				for (auto cmd : Commands)
				{
					std::cout << String::tableFormat({cmd.first, cmd.second}, {l, lineWidth - l}) << "\n";
				}
			}
		}
		TitlePrint(Name, lineWidth);
		std::sort(Fields.begin(), Fields.end());

		for (auto f : Fields)
		{

			std::cout << JSL::String::tableFormat({std::get<0>(f), std::get<1>(f), std::get<2>(f)}, {l, m, remainder});
		}
		std::sort(Nested.begin(), Nested.end(), [](auto &lhs, auto &rhs) { return lhs.Name < rhs.Name; });
		for (auto n : Nested)
		{
			n.Print();
		}
	}
} // namespace JSL::Interface::internal
