#pragma once
#include "Field.h"
#include "Help.h"
#include "JSL/IO/FileWriters.h"
#include "JSL/Interface/CommandLine.h"
#include "JSL/Interface/Config.h"
#include <filesystem>
#include <string>
namespace JSL::Interface
{
	//! Detects if an Aggregator has been correctly assigned a FieldList member
	template <typename T>
	concept HasFieldList = requires(T t) {
		t.FieldList([](auto &&) {});
	};

	//! A CRTP class for turning a POD into a fully configurable entity using the ConfigurableCommandLine protocol
	template <class Derived>
	class Aggregator
	{
	  public:
		//! The name given to this settings block when displayed on the Help Menu
		std::string Name = "Unnamed Settings Block";

		//! A list of Commands parsed by the CommandLine
		std::vector<std::string> Commands;

		//! Metadata associated with the visual display when in the Help Menu
		HelpMetaData HelpData;

		/*!	The 'activation function' which reads from the program input and any config files, and hten automatically parses the output into the typed member variables
		 * @param argc The number of arguments to pass to the command line
		 * @param argv The argument array
		 */
		std::vector<std::string> &Parse(int argc, char **argv)
			requires HasFieldList<Derived>
		{
			CheckInitialised();
			CapturedName = argv[0];
			auto cmd = ConfigurableCommandLine(argc, argv, Map);
			Parse(cmd);
			Commands = cmd.Commands;

			auto it = std::find(Commands.begin(), Commands.end(), "help");
			if (cmd.Parse("help", false) || it != Commands.end())
			{
				Help();
				exit(0);
			}

			return Commands;
		}

		/*!	The 'activation function' which reads from the program input and any config files, and hten automatically parses the output into the typed member variables
		 * @param argc The number of arguments to pass to the command line
		 * @param argv The argument array
		 */
		void Configure(std::filesystem::path file, std::string delim)
			requires HasFieldList<Derived>
		{
			CheckInitialised();
			auto cnf = Config(file, delim, Map);
			Parse(cnf);
		}
		void Configure(std::vector<std::string> fileLines, std::string delim)
			requires HasFieldList<Derived>
		{
			CheckInitialised();
			auto cnf = Config(fileLines, delim, Map);
			Parse(cnf);
		}

		//! Resets all member variables to their default value
		void Reset()
			requires HasFieldList<Derived>
		{
			static_cast<Derived *>(this)->FieldList([&](auto &&field) {
				using E = std::remove_cvref_t<decltype(field)>;
				static_assert(IsField<E> || HasFieldList<E>,
					"FieldList() entry is neither a Field<T> nor an Aggregator: "
					"did you forget to define FieldList() on a nested type?");
				if constexpr (HasFieldList<E>)
				{
					field.Reset(); // recurse
				}
				else
				{
					field.Value = field.Default;
				}
			});
		}
		//! Searches for a value matching the key, and if present, returns the string representation of its current value
		//! @param key A usual aggregator alias key
		//! @return An optional value containing the value (if found), or nullopt if not found
		std::optional<std::string> GetString(std::string key)
			requires HasFieldList<Derived>
		{
			return ScanForString(key);
		}

		/*!	Displays a detailed help message including documentation for all member variables
		 * @details Automatically activated whenever "-h", "-help" or the "help" command are passed into the CommandLine
		 * @param isConsole Set to true when being printed from the console module (rather than the command line). Changes the appearance to match a 'live' view, rather than a 'default' view.
		 */
		void Help(bool isConsole = false)
			requires HasFieldList<Derived>
		{
			size_t maxLWidth = 0;
			size_t maxMWidth = 0;
			CheckInitialised();
			internal::HelpGroup help(Name, &maxLWidth, &maxMWidth);

			if (!isConsole)
			{
				bool spoof = false;
				std::string spoofstring = "";
				HelpData.Commands["help"] = "Activates the help display, then exits (equivalent to -h)";
				Field<bool> helpfield{spoof, "(Inbuilt)", {"h", "help"}, false, "If true, shows the help screen, then quits"};
				Field<std::string> helpconfig{spoofstring, "(Inbuilt)", {"config"}, "-none-", "If a file is passed to this argument, it is used to configure the remaining parameters, at a lower priority than values set from the command line"};
				Field<std::string> helpconfigdelim(spoofstring, "(Inbuilt)", {"config-delim"}, "' '", "This value is used to determine the delimiter used to separate key-value pairs in config files.");
				help.AddField(helpfield, isConsole);
				help.AddField(helpconfig, isConsole);
				help.AddField(helpconfigdelim, isConsole);
				help.AddCommands(HelpData.Commands);
				HelpData.CallingName = CapturedName;
			}

			Help(help, isConsole);
			help.Print(HelpData);
		}

		//! Constructs a config file which can reconstruct the current state of the aggregator
		//! @param excludeDefaults If true, does not include values which are equal to their default (useful for only tracking values which have been altered)
		void Export(std::filesystem::path target, bool excludeDefaults = false)
			requires HasFieldList<Derived>
		{
			JSL::IO::writeString(target, ExportAsString(excludeDefaults));
		}

		//! Creates a string equal to the contents of the equivalent config file
		//! @param excludeDefaults If true, does not include values which are equal to their default (useful for only tracking values which have been altered)
		//! @returns A string, which if saved to file, could be read in as a config file.
		std::string ExportAsString(bool excludeDefaults = false)
		{
			std::ostringstream os;
			Export(os, excludeDefaults);
			return os.str();
		}

		//! Gets a copy of the current key-context
		//! @returns A copy of the internal member Map
		ContextMap GetContext()
		{
			return Map;
		}
		template <class OtherDerived>
		friend class Aggregator;

	  protected:
		//! The name stored in argv[0], used for display purposes
		std::string CapturedName;

		//! Set to true when the context map has been constructed
		bool Initialised = false;

		//! Performs the initialisation steps
		//! @details Would be part of a default constructor, but has to run *after* the child class has been fully declared
		void CheckInitialised()
		{
			if (Initialised) return;
			Reset();
			BuildContext();
			Initialised = true;
		}

		//! Constructs the ContextMap from the FieldList
		//! @details This is the version called by the 'Root aggregator', blank-initialising the Map which will be passed down the heirarchy
		void BuildContext()
			requires HasFieldList<Derived>
		{
			Map = {};
			BuildContext(Map);
		}
		//! Constructs the ContextMap from the FieldList
		//! @details This is the version which descends through the full aggregator heirarchy, filling in the context map that the Root object holds
		void BuildContext(ContextMap &map)
		{
			static_cast<Derived *>(this)->FieldList([&](auto &&field) {
				using E = std::remove_cvref_t<decltype(field)>;
				static_assert(IsField<E> || HasFieldList<E>,
					"FieldList() entry is neither a Field<T> nor an Aggregator: "
					"did you forget to define FieldList() on a nested type?");
				if constexpr (HasFieldList<E>)
				{
					field.BuildContext(map); // recurse
					field.BuildContext();	 // for reach-in options
					field.Initialised = true;
				}
				else
				{
					map.AddContext(Context(field.Aliases, InferKeyType(field.Value)));
				}
			});
		}

		//! Pipes the contents of the equivalent config file
		//! @param os An output stream used to store the results
		//! @param excludeDefaults If true, does not include values which are equal to their default (useful for only tracking values which have been altered)
		void Export(std::ostringstream &os, bool excludeDefaults = false)
		{
			static_cast<Derived *>(this)->FieldList([&](auto &&field) {
				using E = std::remove_cvref_t<decltype(field)>;
				static_assert(IsField<E> || HasFieldList<E>,
					"FieldList() entry is neither a Field<T> nor an Aggregator: "
					"did you forget to define FieldList() on a nested type?");
				if constexpr (HasFieldList<E>)
				{
					field.Export(os, excludeDefaults);
				}
				else
				{
					if (!excludeDefaults || field.Value != field.Default)
					{
						os << field.Aliases[0] << " " << JSL::String::makeFrom(field.Value) << "\n";
					}
				}
			});
		}

		//! Iterates over the FieldList using the CommandLine estbalished by the root object
		//! @param cmd A commandline which has already tokenised its input
		void Parse(ParserBase &cmd)
			requires HasFieldList<Derived>
		{
			static_cast<Derived *>(this)->FieldList([&](auto &&field) {
				using E = std::remove_cvref_t<decltype(field)>;
				static_assert(IsField<E> || HasFieldList<E>,
					"FieldList() entry is neither a Field<T> nor an Aggregator: "
					"did you forget to define FieldList() on a nested type?");
				if constexpr (HasFieldList<E>)
				{
					field.Parse(cmd); // recurse
				}
				else
				{
					field.Value = cmd.Parse(field.Aliases[0], field.Value);
				}
			});
		}

		//! The internal help function, which populates the HelGroup held by the main object
		//! @param help A storage and formatting device
		//! @param isConsole Changes if the display is for the 'live console' view (true) or the 'default CLI view' (false)
		void Help(internal::HelpGroup &help, bool isConsole)
		{

			static_cast<Derived *>(this)->FieldList([&](auto &&field) {
				using E = std::remove_cvref_t<decltype(field)>;
				static_assert(IsField<E> || HasFieldList<E>,
					"FieldList() entry is neither a Field<T> nor an Aggregator: "
					"did you forget to define FieldList() on a nested type?");
				if constexpr (HasFieldList<E>)
				{
					auto &nested = help.NestGroup(field.Name);
					field.Help(nested, isConsole); // recurse
				}
				else
				{
					help.AddField(field, isConsole);
				}
			});
		}

		std::optional<std::string> ScanForString(std::string_view key)
		{
			std::optional<std::string> out = std::nullopt;
			static_cast<Derived *>(this)->FieldList([&](auto &&field) {
				using E = std::remove_cvref_t<decltype(field)>;
				static_assert(IsField<E> || HasFieldList<E>,
					"FieldList() entry is neither a Field<T> nor an Aggregator: "
					"did you forget to define FieldList() on a nested type?");
				if constexpr (HasFieldList<E>)
				{
					if (auto nested = field.ScanForString(key)) // recurse
					{
						out = nested;
					}
				}
				else if (std::find(field.Aliases.begin(), field.Aliases.end(), key) != field.Aliases.end())
				{
					out = JSL::String::makeFrom(field.Value);
				}
			});

			return out;
		}

		ContextMap Map = {};
	};
} // namespace JSL::Interface
