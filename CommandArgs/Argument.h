#pragma once
#include <string>
#include <stdexcept>
#include <math.h>
#include "../FileIO/FileIO.h"
#include <sstream>

namespace JSL
{
	/*!
		A wrapper class for all Argument objects, enabling heterogenous argument lists etc. to be constructed. 	
		Contains the virtual methods common to all objects. All Argument objects are assumed to take name-value pairs, with the associated name indicated by the #TriggerString
	*/
	class ArgumentInterface
	{
		public:
			
			ArgumentInterface(std::string string) : TriggerString(string) {};
			 
			//!Virtual alias for Argument::Parse()
			virtual void Parse( char * name, char * value){};
			
			//!Virtual alias for Argument::ListParse()
			virtual void ListParse( int argc, char * argv[]){};
			
			//!Virtual alias for Argument::Configure()
			virtual void Configure(std::string configFile, char configDelimiter){};
			
			//!The chosen "Name" of the Argument - the string which will trigger the Parse() function to write in the passed value
			const std::string TriggerString;
		
	};
	
	
	
	/*!
		A class which allows arbitrary template parameters to be read in as command-line arguments or from a configuraiton file using a Name/Value pair system. Upon construction, the #Value parameter takes the default value (if provided), until it is overriden by a successful argument/#TriggerString match.
	*/
	template <class T>
	class Argument : public ArgumentInterface
	{
		
		public:
			//!The current value of the argument. 
			T Value;
		
			//!Default constructor
			Argument(){};
			
			//!Constructor initialising the #TriggerString. Value is set to uninitialised memory of the Template type. \param trigger The value of #TriggerString, and the "name" of this parameter 
			Argument(std::string trigger) : ArgumentInterface(trigger)
			{
				CheckForInvalidTriggers();
			}
			
			//!Constructor which initialises the #TriggerString and #Value members. \param defaultValue The initialisation value of #Value - overridden if Parse() is called. \param trigger The value of #TriggerString, and the "name" of this parameter 
			Argument(T defaultValue,std::string trigger) : ArgumentInterface(trigger)
			{
				Value = defaultValue;
				//~ TriggerString = trigger;
				CheckForInvalidTriggers();
			}
			
			//!Constructor which initialises as per #Argument(T defaultValue, std::string trigger), but which also immediately calls ListParse() to check for assignment \param defaultValue The initialisation value of #Value - overridden if Parse() is called. \param trigger The value of #TriggerString, and the "name" of this parameter \param argc The number of commandline arguments \param argv[] the command line list
			Argument(T defaultValue, std::string trigger, int argc, char * argv[]) : ArgumentInterface(trigger)
			{
				Value = defaultValue;
				//~ TriggerString = trigger;
				
				ListParse(argc,argv);
				CheckForInvalidTriggers();
			}
			
			//!Constructor which initialises as per #Argument(T defaultValue, std::string trigger), but which also immediately calls Configure() to check for assignment \param defaultValue The initialisation value of #Value - overridden if Parse() is called. \param trigger The value of #TriggerString, and the "name" of this parameter \param configFile The path to the file to open and parse for configuration data \param configDelimiter The delimiter used to separate Name/Value pairs in the cofiguration file
			Argument(T defaultValue, std::string trigger, std::string configFile, char configDelimiter) : ArgumentInterface(trigger)
			{
				Value = defaultValue;
				//~ TriggerString = trigger;
				Configure(configFile, configDelimiter);
				CheckForInvalidTriggers();
			}
			
			//!Iterate through a configuration file, extracting Name/Value pairs and calling Parse() in them. Each Name/Value pair should be on a new line in the file, and separated by the *configDelimiter*. \param configFile The path to the file to open and parse for configuration data \param configDelimiter The delimiter used to separate Name/Value pairs in the cofiguration file
			void Configure(std::string configFile, char configDelimiter)
			{
				forLineVectorIn(configFile, configDelimiter,
					if (FILE_LINE_VECTOR.size() > 1 && FILE_LINE_VECTOR[0] == TriggerString)
					{
						AssignValue(FILE_LINE_VECTOR[1].c_str());
					}
				);
			}
			
			//!Iterate through the provided commandline args, extracting Name/Value pairs and calling Parse() on them. \param argc The number of arguments passed to the program \param argv[] The argument list (argv[0] is assumed to be the the name of the program, and is ignored)
			virtual void ListParse( int argc, char * argv[])
			{
				for (int i = 1; i < argc-1; ++i)
				{
					Parse(argv[i],argv[i+1]);
				}
			}
			
			//! Checks if the Name matches the TriggerString. For a successful match, Name must be prefaced by one more dash than found in TriggerString. \param name The name-string of the Name/Value pair\param value The value-string of the Name/Value pair. 
			void Parse( char * name, char * value)
			{
				std::string sArg = name;
				if (sArg == "-" + TriggerString)
				{
					AssignValue(value);
				}
			}
	
			//! Allow the Argument object to be implicitly casted into the value of #Value, and hence treated as an object of the templated type.
			operator T()
			{
				return Value;
			}
			
			//! Annoying const version
			operator T() const
			{
				return Value;
			}
			
		private:
			//!Some Triggers are disallowed - they usually are protected names such as "help", though other properties may trigger this funciton to throw an error.
			
			void CheckForInvalidTriggers()
			{
				std::vector<std::string> ProtectedStrings = {"help"};
				
				for (int i = 0; i < ProtectedStrings.size(); ++i)
				{
					if (TriggerString == ProtectedStrings[i])
					{
						throw std::invalid_argument(TriggerString + " is a protected triggername");
					}
				}
			}
			

			
			//!Virtual override for template-specific AssignValue calls. Most template types will require a custom handler to convert value into the chosen template type -- some default ones are provided below.
			virtual void AssignValue( const char * value){};
			
	};
	
	//!Override of the AssignValue() function for Argument<double> objects. Throws an error if the value is a non-integer, to prevent silent casting/truncation
	template<>
	inline void Argument<int>::AssignValue( const char * value)
	{
		double testDouble = std::stod(value);
		int testInt = (int)testDouble;
		
		if ((double)testInt != testDouble)
		{
			throw std::invalid_argument("Argument passed to " + TriggerString + " was a double, expected an integer");
		}
		
		//this method ensures that it can interpret in exponential notation as stod can do that!
		Value = testInt;
	}
	//!Override of the AssignValue() function for Argument<long long int> objects. Throws an error if the value is a non-integer, to prevent silent casting/truncation
	template<>
	inline void Argument<long long int>::AssignValue( const char * value)
	{
		double testDouble = std::stod(value);
		long long int testInt = (long long int)std::stod(value);
		
		if ((double)testInt != testDouble)
		{
			throw std::invalid_argument("Argument passed to " + TriggerString + " was a double, expected an integer");
		}
		
		//this method ensures that it can interpret in exponential notation as stod can do that!
		Value = testInt;
	}
	
	
	//!Override of the AssignValue() function for Argument<double> objects
	template<> 
	inline void Argument<double>::AssignValue(const char * value)
	{
		Value = std::stod(value);
	}
	
	//!Override of the AssignValue() function for Argument<std::string> objects
	template<>
	inline void Argument<std::string>::AssignValue(const char * value)
	{
		Value = (std::string)value;
	}

	//!Override of the AssignValue() function for Argument<char> objects
	template<>
	inline void Argument<char>::AssignValue(const char * value)
	{
		Value = (char)value[0];
	}
	
	

	
	//!Override of the AssignValue() function for Argument<bool> objects. Accepts only 0/1 as valid bool-strings
	template<>
	inline void Argument<bool>::AssignValue( const char * value)
	{
		int testValue = std::stoi(value);
		if (testValue != 0 && testValue != 1)
		{
			throw std::runtime_error("Argument passed to " + TriggerString + " was not a bool (0 or 1)");
		}
		Value = (bool)testValue;
	}
	
	//!Provides an explicit instruction for how to infer outputting the arg-value. This works (temperamentally) without this function, but strings (well-known for being fiddly here) can sometimes mess it up, so providing explicit instruction works for the best.
	template<class T>
	std::ostream & operator<<(std::ostream & os, const Argument<T> a)
	{
		os << a.Value;
		return os;
	}

}
