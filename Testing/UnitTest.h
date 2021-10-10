#pragma once
#include "string.h"
#include <iostream>

namespace JSL
{
	
	/*!
	 * A parent class for various unit testing frameworks. Provides a consistent API for interacting with the unit tests. The class is essentially meaningless on its own, but when coupled to a decently written subclass (i.e. JSL_Testing::StringTest), it becomes very useful.
	*/
	class UnitTest
	{
		public:
			//! The name by which the UnitTest refers to itself. This should be fairly descriptive, to help locating the failing unit test (don't give them all the same name!)
			std::string Name;
		
			//! If true, the UnitTest is considered successful, and no errors are thrown
			bool Passed;
			
			//! If true, the UnitTest will only throw warnings, rather than exist, when an error is encountered. Default is false. 
			bool NonCriticalFailure;
			
			//! A message buffer which is output when Results() is called, giving updates on the progress of the test.
			std::vector<std::string> MessageBuffer;
			
			
			//! Constructor class. Initialises the system to a failing state.
			UnitTest()
			{
				Passed = false;
				NonCriticalFailure = true;
				Name = "Unnamed";
				MessageBuffer = {"Uninitialised testing procedure -- this is not a well-formatted test"};
				
			}
			
			//! All children which want to take advantage of the BufferedTest() command should have the bulk of their testing within this command.
			virtual void Run_Test()
			{
				
			}
			
			//! Calls the overridden function Run_Test() within a try-catch loop. Provides an extra level of safety and diagnostics for any unhandled exceptions.
			void BufferedTest()
			{
				try 
				{
					Run_Test();
				}
				catch (const std::exception& e)
				{
					Passed = false;
					MessageBuffer.push_back("An unhandled runtime error occured with message: \n\t" + (std::string)e.what());
				}
			}
			
			//! Prints the results of the testing to the terminal and prints the contents of MessageBuffer line-by-line \return The value of Passed (true if test succeeded)
			bool Results()
			{
				std::cout  << "\n" << Name;
				if (Passed)
				{
					std::cout << " Test Sequence was successful. " << std::endl;
					for (auto message : MessageBuffer)
					{
						std::cout << "\t" << message << std::endl;
					}
				}
				else
				{
					std::cout << " Test Sequence failed with error message:"<< std::endl;
					for (auto message : MessageBuffer)
					{
						std::cout << "\t" << message << std::endl;
					}
					if (NonCriticalFailure)
					{
						std::cout<< "\n\tThis is a non-critical failure.";
					}
					else
					{
						std::cout <<"\n\tThis is judged to be a critical failure. Immediate termination will ensue" << std::endl;
						exit(-1);
					}
				}
				return Passed;
			}	
		protected:
			/*!
			 * A structure to help basic logic flow within unit tests. If the condition is false, the unit test considers itself failed, and updates the messages and Passed status accordingly
			 * \param condition If true, the sub-test is successful
			 * \param successmessage If condition == true, this is the message appended to the MessageBuffer
			 * \param failuremessage If condition == false, this message is used instead.
			*/
			void basicConditionCheck(bool condition, std::string successmessage, std::string failuremessage)
			{
				if (condition)
				{
					MessageBuffer.push_back(successmessage);
				}
				else
				{
					MessageBuffer.push_back("ERROR: " + failuremessage);
					Passed = false;
				}
			}
			
			//! An override of the other function, preventing dual-typing of basic output messages
			void basicConditionCheck(bool condition, std::string messageFramework)
			{
				std::string success = messageFramework + " was successful";
				std::string failure = messageFramework + " failed";
				basicConditionCheck(condition,success,failure);
			}
	};
}
