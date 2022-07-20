#pragma once

#include "Argument.h"
namespace JSL
{
	const bool DEFAULT_TOGGLE_VALUE = false;
	
	//!A Toggle is a special case of an Argument which doesn't actually require an argument - that is, the presence of the flag is informative. A toggle is an Argument<bool> object with a specialised constructor which looks only for the presence of the TriggerString.
	class Toggle : public Argument<bool>
	{
		// Toggle() : Argument<bool>(){};
		public:
			//!Non-Parsing Constructor. With no default value provided, false is assumed.
			Toggle(std::string trigger) : Argument<bool>(DEFAULT_TOGGLE_VALUE, trigger) {};
			//!Non-Parsing Constructor
			Toggle(bool defaultVal, std::string trigger) : Argument<bool>(defaultVal,trigger){};
			//!Parsing Constructor
			Toggle(bool defaultVal, std::string trigger, int argc, char ** argv) : Argument<bool>(defaultVal,trigger)
			{
				ToggleParse(argc,argv);
			};
			//!Parsing Constructor With no default value provided, false is assumed (hence True if trigger is in argv)
			Toggle(std::string trigger, int argc, char ** argv) : Argument<bool>(DEFAULT_TOGGLE_VALUE,trigger)
			{
				ToggleParse(argc,argv);
			};
		private:
			//!Overload the usual Argument::ListParse with a specialised version for detecting single-arg elements.
			void ToggleParse(int argc, char * argv[])
			{
				for (int i = 1; i < argc; ++i)
				{
					if ((std::string)argv[i] == "-" + TriggerString)
					{
						Value = !Value;
						break;
					}
				}
			}
	};
}