#include <JSL/Display/Format.h>
#include <JSL/Display/Terminal.h>
#include <JSL/Interface/Aggregator.h>
#include <JSL/Interface/old_reader.h>
#include <JSL/Strings/Wrap.h>
#include <JSL/Vectors/Search.h>
namespace JSL::Parameter
{

	std::string cmdCapture;
	Aggregator::Aggregator()
	{
		Connect(HelpToggle, Help);
		Set(Help, HelpToggle, false, {"h", "help"}, "Help", "Equivalent to the help command");
		AddCommand("help", "Activates the helper function, prints out usage, commands and options, then exits without further computation.");
	}
	Aggregator::Aggregator(std::string_view name) : Aggregator()
	{
		Name = static_cast<std::string>(name);
	}
	void Aggregator::Parse(int argc, char **argv)
	{
		cmdCapture = std::string{argv[0]};
		Parameter::NestedAggregator::Parse(argc, argv);
		auto cmds = InvokedCommands();
		if (Help || JSL::Vector::contains(cmds, "help"))
		{
			HelpMenu();
			std::exit(0);
		}
	}

	template <class T, class U>
	std::string fmtwrap(T prefix, std::string text, U suffix)
	{
		if (!Display::Terminal().IsANSICapable()) { return text; }

		return (std::string)prefix + text + suffix;
	}
	template <class T>
	std::string fmtwrap(T prefix, std::string text)
	{
		if (!Display::Terminal().IsANSICapable()) { return text; }

		return (std::string)prefix + text + Display::ResetAll();
	}

	void Aggregator::HelpMenu()
	{
		std::cout << "Usage:\n";
		std::cout << "\t" << cmdCapture << " [commands] [options]\n";
		std::cout << Display::Colour(40, 40, 40) << Display::Italics();
		std::cout << "Commands are any space separated commands before the first option\nOptions must be indicated by at least one dash\n";

		// commands are printed only once
		auto cmds = RegisteredCommands();
		if (!cmds.empty())
		{
			printAsTitle("Available Commands");
			auto defaulter = GetDefaultCommand();
			for (auto &[name, txt] : cmds)
			{
				std::string suffix = "";
				if (name == defaulter)
				{
					suffix = fmtwrap(Display::Colour(50, 50, 50), " (default)");
				}
				std::string title = fmtwrap(Display::Bold() + Display::Green(), name);
				std::cout << String::tableFormat({title + suffix, txt}, {lw, mw + rw}, " ") << "\n";
			}
		}

		PrintHelp(Name);
	}
	std::vector<std::string> Parameter::Aggregator::InvokedCommands()
	{
		auto &instance = internal::Parameter::CommandLineReader::Get();
		return instance.Commands;
	}
	std::string &Parameter::Aggregator::GetDefaultCommand()
	{
		static std::string instance = "";
		return instance;
	}
	std::map<std::string, std::string> &Parameter::Aggregator::RegisteredCommands()
	{
		static auto instance = std::map<std::string, std::string>{};
		return instance;
	}
	void Parameter::Aggregator::DefaultCommand(std::string name, std::string function)
	{
		AddCommand(name, function);
		auto &r = GetDefaultCommand();
		r = name;
	}
	void Parameter::Aggregator::AddCommand(std::string name, std::string function)
	{
		RegisteredCommands().try_emplace(name, function);
	}

	std::pair<std::set<std::string>, std::set<std::string>> Parameter::Aggregator::ParseCommands()
	{
		std::set<std::string> registered;
		std::set<std::string> unregistered;

		auto expected = RegisteredCommands();
		for (auto cmd : InvokedCommands())
		{
			if (expected.contains(cmd))
			{
				registered.insert(cmd);
			}
			else
			{
				unregistered.insert(cmd);
			}
		}
		return {registered, unregistered};
	}

} // namespace JSL::Parameter
