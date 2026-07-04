#pragma once
// TODO: Then it all needs documenting
#include "Field.h"
#include "Help.h"
#include "JSL/Interface/CommandLine.h"
#include <string>
namespace JSL::Interface
{

	template <typename T>
	concept HasFieldList = requires(T t) {
		t.FieldList([](auto &&) {});
	};

	template <class Derived>
	class Aggregator
	{
	  public:
		std::string Name = "Unnamed Settings Block";
		std::vector<std::string> Commands;
		HelpMetaData HelpData;
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

		void Help()
			requires HasFieldList<Derived>
		{
			CheckInitialised();
			internal::HelpGroup help(Name);
			bool spoof = false;
			std::string spoofstring = "";
			HelpData.Commands["help"] = "Activates the help display, then exits (equivalent to -h)";
			Field<bool> helpfield{spoof, "(Inbuilt)", {"h", "help"}, false, "If true, shows the help screen, then quits"};
			Field<std::string> helpconfig{spoofstring, "(Inbuilt)", {"config"}, "-none-", "If a file is passed to this argument, it is used to configure the remaining parameters, at a lower priority than values set from the command line"};
			Field<std::string> helpconfigdelim(spoofstring, "(Inbuilt)", {"config-delim"}, "' '", "This value is used to determine the delimiter used to separate key-value pairs in config files.");
			help.AddField(helpfield);
			help.AddField(helpconfig);
			help.AddField(helpconfigdelim);
			HelpData.CallingName = CapturedName;
			help.AddCommands(HelpData.Commands);
			Help(help);
			help.Print(HelpData);
		}
		template <class OtherDerived>
		friend class Aggregator;

	  protected:
		std::string CapturedName;
		bool Initialised = false;
		void CheckInitialised()
		{
			if (Initialised) return;
			Reset();
			BuildContext();
			Initialised = true;
		}
		void BuildContext()
			requires HasFieldList<Derived>
		{
			Map = {};
			BuildContext(Map);
		}
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
				}
				else
				{
					map.AddContext(Context(field.Aliases, InferKeyType(field.Value)));
				}
			});
		}

		void Parse(ConfigurableCommandLine &cmd)
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
		void Help(internal::HelpGroup &help)
		{

			static_cast<Derived *>(this)->FieldList([&](auto &&field) {
				using E = std::remove_cvref_t<decltype(field)>;
				static_assert(IsField<E> || HasFieldList<E>,
					"FieldList() entry is neither a Field<T> nor an Aggregator: "
					"did you forget to define FieldList() on a nested type?");
				if constexpr (HasFieldList<E>)
				{
					auto &nested = help.NestGroup(field.Name);
					field.Help(nested); // recurse
				}
				else
				{
					help.AddField(field);
				}
			});
		}
		ContextMap Map = {};
	};
} // namespace JSL::Interface
