#pragma once
#include "Aggregator.h"
#include <JSL/Display/Format.h>
#include <iostream>
namespace JSL::Interface
{
	//! @brief A non-templated helper object used for the Console routine
	struct ConsoleHelper
	{
	  public:
		//! If true, a syntax description is placed at the beginning of the Console routine
		bool ShowIntro = true;

		//! The colour applied to 'example commands', and to the user input
		JSL::Display::Format CmdColor = JSL::Display::Cyan();

		//! The string which will be used as the exit command
		std::string ExitString = "exit";

		//! The string which will be used as the key-overwrite command
		std::string SetString = "set";

		//! The string which will be used as the vector-add command
		std::string AddString = "add";
		//! The string which will be used as the vector-remove command
		std::string RemoveString = "remove";

		//! The string which will be used as the show-value command
		std::string ShowString = "show";

		//! The string which will be used as the show-help command
		std::string HelpString = "help";

		//! The string output when the user enters a non-blank command other that set, show etc.
		std::string UnrecognisedCommand = "Unrecognised Command";

		//! If true, the commands are case-insensitive
		bool caseInsensitive = true;

		//! Prints ">> " + modifies the colour at the start of each command prompt
		void LoopPrompt();

		//! If ShowIntro is true, prints the syntax menu to screen
		void InitialPrompt();

		//! A wrapper for LOG(WARN) to avoid linking header files too tightly
		//! @param string The warning message to be printed
		void Warn(std::string_view string);

		//! A function which prints using the LOG(INFO) syntax, but avoids the conditional check, so prints at any logging level
		//! @detail This is to be used for info-like statements that the user *asked* for, and so shouldn't be hidden at WARN or ERROR levels
		//! @param string The message to be printed
		void Print(std::string_view string);

		//! A POD for holding the tokenisation information
		struct CmdTokens
		{
			//! Set to true when the input command is equal to ExitString
			bool ExitCommand;
			//! The extracted Command, which must match SetString, ShowString etc
			std::string Command;
			//! The argument to be passed on
			std::string Argument;
		};
		void Unformat();
		//! Parses a user provided command, and tokenises it
		//! @param command The user input
		//! @param context The context map of the Aggregator being manipulated
		//! @return An object detailing what actions are to be taken next
		CmdTokens Tokenise(std::string command, ContextMap context);

		std::pair<std::string, std::string> VectorExtract(std::string input);
		std::string ConstructReducedVector(std::string original, std::string element);

	  private:
		CmdTokens internalTokenise(std::vector<std::string> tokens, ContextMap &context, bool hasInverted);
	};

	namespace internal
	{

		/*! The function which replaces the old value with the new user provided one
		 * 	@param input The aggregator being modified
		 * 	@param helper The helper module
		 * 	@param cmd 	The user command (which should match a config-file command)
		 */
		template <class T>
		void SetKey(Aggregator<T> &input, ConsoleHelper &helper, std::string &cmd)
		{
			try
			{
				input.Configure(std::vector<std::string>{cmd}, " ");
			}
			catch (std::exception &e)
			{
				helper.Warn("Could not parse input: " + std::string(e.what()));
			}
		}
		/*! The function which appends the the new user provided value into the new old one
		 * 	@param input The aggregator being modified
		 * 	@param helper The helper module
		 * 	@param cmd 	The user command
		 */
		template <class T>
		void AppendKey(Aggregator<T> &input, ConsoleHelper &helper, std::string &cmd)
		{
			try
			{
				auto [key, val] = helper.VectorExtract(cmd);
				if (!key.empty())
				{
					auto old = input.GetString(key);
					if (!old)
					{
						helper.Warn("Could not extract current value of " + key);
						return;
					}
					std::string oldStr = old.value();
					if (oldStr.back() == ']')
					{
						oldStr = oldStr.substr(1, oldStr.size() - 2);
					}
					input.Configure(std::vector<std::string>{key + " " + oldStr + ", " + val}, " ");
				}
			}
			catch (std::exception &e)
			{
				helper.Warn("Could not parse input: " + std::string(e.what()));
			}
		}
		/*! The function which removes a single chosen element from the user chosen vector
		 * 	@param input The aggregator being modified
		 * 	@param helper The helper module
		 * 	@param cmd 	The user command
		 */
		template <class T>
		void RemoveKey(Aggregator<T> &input, ConsoleHelper &helper, std::string &cmd)
		{
			try
			{
				auto [key, val] = helper.VectorExtract(cmd);
				if (!key.empty())
				{
					auto old = input.GetString(key);
					if (!old)
					{
						helper.Warn("Could not extract current value of " + key);
						return;
					}
					std::string oldStr = old.value();
					if (oldStr.back() == ']')
					{
						oldStr = oldStr.substr(1, oldStr.size() - 2);
					}
					auto newVec = helper.ConstructReducedVector(oldStr, val);

					if (!newVec.empty())
					{
						input.Configure(std::vector<std::string>{key + " " + newVec}, " ");
					}
				}
			}
			catch (std::exception &e)
			{
				helper.Warn("Could not parse input: " + std::string(e.what()));
			}
		}
		/*! The function which displays the current value
		 * 	@param input The aggregator being modified
		 * 	@param helper The helper module
		 * 	@param cmd 	The user-provided key (which we know exists at this point)
		 */
		template <class T>
		void ShowKey(Aggregator<T> &input, ConsoleHelper &helper, std::string &cmd)
		{
			auto val = input.GetString(cmd);
			if (val)
			{
				helper.Print(cmd + " = " + val.value());
			}
			else
			{
				helper.Warn("Cannot retrieve key " + cmd);
			}
		}

	} // namespace internal

	/*! Initialises a (blocking) user interface which allows the user to modify or view variables in the selected Aggregator using the same syntax as in a config file.
	 * @param aggregator A JSL::Interface::Aggregator object which will act as the baseline
	 * @param helper A settings-object which determines the behaviour of the interface
	 * @returns A copy of the input aggregator, modified to the user's specifications.
	 */
	template <class T>
		requires std::derived_from<T, Aggregator<T>> && HasFieldList<T>
	T Console(T aggregator, ConsoleHelper helper = {})
	{
		helper.InitialPrompt();
		bool continues = true;
		std::string command;
		while (continues)
		{
			helper.LoopPrompt();

			if (!std::getline(std::cin, command))
			{
				helper.Unformat();
				helper.Warn("Input stream closed early");
				return aggregator;
			}
			helper.Unformat();
			auto tokens = helper.Tokenise(command, aggregator.GetContext());
			if (tokens.ExitCommand)
			{
				continues = false;
			}
			else if (tokens.Command == helper.HelpString)
			{
				aggregator.Help(true);
			}
			else if (tokens.Command == helper.SetString)
			{
				internal::SetKey(aggregator, helper, tokens.Argument);
			}
			else if (tokens.Command == helper.AddString)
			{
				internal::AppendKey(aggregator, helper, tokens.Argument);
			}
			else if (tokens.Command == helper.RemoveString)
			{
				internal::RemoveKey(aggregator, helper, tokens.Argument);
			}
			else if (tokens.Command == helper.ShowString)
			{
				internal::ShowKey(aggregator, helper, tokens.Argument);
			}
		}

		return aggregator;
	}
} // namespace JSL::Interface
