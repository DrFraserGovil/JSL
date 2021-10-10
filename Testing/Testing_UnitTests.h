#pragma once

#include "../JSL.h"
#include <cmath>
namespace JSL_Testing
{
	class MetaTest : public JSL::UnitTest
	{
		public:
			MetaTest() : JSL::UnitTest()
			{
				Name = "JSL::UnitTest";
				
				Run_Test();
			}
			
			void Run_Test()
			{
				std::vector<std::string> MetaMessage;
				bool goodMetaTest = true;
				
				if (Passed != false)
				{
					MetaMessage.push_back("Baseline Pass initialisation failed");
					goodMetaTest = false;
				}
				
				if (MessageBuffer[0].length() == 0)
				{
					MetaMessage.push_back("Baseline string initialisation failed");
					goodMetaTest = false;
				}
				
				if (!NonCriticalFailure)
				{
					MetaMessage.push_back("Default critical analysis is set to exit");
					goodMetaTest = false;
				}
				
				Passed = goodMetaTest;
				NonCriticalFailure = false;
				if (Passed)
				{
					MessageBuffer[0] = "The baseline UnitTest structure is operating as expected";
				}
				else
				{
					MessageBuffer = MetaMessage;
				}
			}
			
	};

	
	class StringTest : public JSL::UnitTest
	{
		public: 

		StringTest()
		{
			Name = "JSL::String";
			NonCriticalFailure = false;
			
			BufferedTest();
			
		}
		

		void Run_Test()
		{
			Passed = true;
			MessageBuffer.resize(0);
			
			//split test
			std::string splitTestString= "1,2,3,4,5,octopus";
			std::vector<std::string> expectedOutput = {"1","2","3","4","5","octopus"};
			std::vector<std::string> output= JSL::split(splitTestString,',');
			bool passedSplitTest = true;
			std::string splitMessage = "JSL::split() functions correctly";
			for (int i = 0; i < output.size(); ++i)
			{
				if (output[i] !=  expectedOutput[i])
				{
					passedSplitTest = false;
					splitMessage = "JSL::split() not functioning as expected";
				}
			}
		
			MessageBuffer.push_back(splitMessage);
			Passed = Passed || passedSplitTest;
			
			//current time test
			bool passedTimeTests = true;
			std::string timeMessage;
			std::string time = JSL::CurrentTime();
			
			
			std::string success = "Current time acquisition returned the following: " + time;
			std::string failure = "Current time acquisition failed";
			basicConditionCheck(time.length() > 0, success,failure);
			
			
			int funDuration = 86468;
			std::string expectedFormatOutput = "1 Day 1 Minute 8 Seconds ";
			
			std::string formatOutput = JSL::FormatDuration(funDuration);
			
			success = "Time formatting working as expected";
			failure = "Time formatting failed: " + formatOutput + "!= " + expectedFormatOutput;
			basicConditionCheck(expectedFormatOutput == formatOutput, success,failure);
		}
		
	};
	
	class IOTest : public JSL::UnitTest
	{
		public:
			IOTest()
			{
				Name = "JSL::FileIO";
				NonCriticalFailure = false;
				BufferedTest();
			}
			
			void Run_Test()
			{
				MessageBuffer.resize(0);
				Passed = true;
				std::string message;
				
				// I believe /. and /.. are guaranteed to exist everywhere...seems the safest way to check that this works
				bool canFindLocality = JSL::locationExists("/.") && JSL::locationExists("/..");
				std::string success ="The location-existence checker identified itself as being in a linux filesystem";
				std::string failure ="The location-existence checker could not identify a basic linux filesystem";
				basicConditionCheck(canFindLocality, success,failure);
				
				
				
				//mkdir checker
				std::string tempDirNameTopLevel = "iotest_tmp";
				std::string nested = "nested/nested2";
				std::string tempDirName = tempDirNameTopLevel + "/" + nested;
				JSL::mkdirReturn output = JSL::mkdir(tempDirName);

				success = "The mkdir routine successfully created " + tempDirName;
				failure = "mkdir Routined failed to create " + tempDirName;
				basicConditionCheck(output.Successful && JSL::locationExists(tempDirName), success,failure);


				
				//initialise file
				std::string fileName = tempDirName + "/test.txt";
				JSL::initialiseFile(fileName);
				
				success = fileName + " was successfully initialised as an empty file";;
				failure = "Could not initialise " + fileName + " as an empty file";
				basicConditionCheck(JSL::locationExists(fileName), success,failure);
				
				
				//file writing -- can use some JSL::String stuff here as we are guaranteed to reach this point only if split() etc. pass their tests
				std::string testString = "test 0 1 2, j z y, 99 29, 100 200 300";
				JSL::writeStringToFile(fileName,testString);
				
				int lines = 0;
				bool linesMatched = true;
				forLineIn(fileName,
					++lines;
					if (FILE_LINE != testString)
					{
						linesMatched = false;
					}
				);
				if (lines > 1)
				{
					linesMatched = false;
				}
				
				success = "JSL::writeStringToFile and forLineIn were able to work together to produce consistent output";
				failure = "JSL::writeStringToFile and forLineIn did not produce consistent output";
				basicConditionCheck(linesMatched, success,failure);

				//vector check
				JSL::initialiseFile(fileName);
				std::vector<std::string> rows = JSL::split(testString,',');
				JSL::writeVectorToFile(fileName,rows,"\n",false);
				lines = 0;
				linesMatched = true;
				forLineIn(fileName,
					++lines;
					if (FILE_LINE != rows[lines-1])
					{
						linesMatched = false;
					}
				);
				if (lines != rows.size())
				{
					linesMatched= false;
				}
				success = "JSL::writeVectorToFile and forLineIn were able to work together to produce consistent output";
				failure = "JSL::writeVectorToFile and forLineIn did not produce consistent output";
				basicConditionCheck(linesMatched, success,failure);


				//matrix check
				JSL::initialiseFile(fileName);
				std::vector<std::vector<std::string>> matrix;
				for (auto row : rows)
				{
					matrix.push_back(JSL::split(row,' '));
				}
				JSL::writeMatrixToFile(fileName,matrix,"|","\n");
				linesMatched = true;
				lines = 0;
				forLineVectorIn(fileName,'|',
					if (FILE_LINE_VECTOR.size() != matrix[lines].size())
					{
						linesMatched = false;
					}
					else
					{
						for (int i = 0; i < FILE_LINE_VECTOR.size(); ++i)
						{
							if (FILE_LINE_VECTOR[i] != matrix[lines][i])
							{
								linesMatched = false;
							}
						}
					}
					++lines;
				);
				if (lines != matrix.size())
				{
					linesMatched = false;
				}
				success = "JSL::writeMatrixToFile and forLineVectorIn were able to work together to produce consistent output";
				failure = "JSL::writeMatrixToFile and forLineVectorIn did not produce consistent output";
				basicConditionCheck(linesMatched, success,failure);
				
				//removal check!
				JSL::rm(tempDirNameTopLevel,true);
				success = "JSL::rm was able to remove the test files";
				failure = "JSL::rm could not remove the test files";
				basicConditionCheck(!JSL::locationExists(tempDirNameTopLevel),success,failure);
			}
	};
	
	class ArgumentTest : public JSL::UnitTest
	{
		public:
			ArgumentTest()
			{
				Name = "JSL::Argument";
				NonCriticalFailure = false;
				MessageBuffer.resize(0);
				BufferedTest();
			}
			void Run_Test()
			{
				Passed = true;
				
				//Basic casting test
				JSL::Argument<int> intArg(3,"int");
				JSL::Argument<double> doubleArg(5.5,"double");
				
				double expectedSum = 3 + 5.5;
				double sum = intArg + doubleArg;
				
				std::string success = "Basic casting methods successful";
				std::string failure = "Basic casting methods failed";
				basicConditionCheck(sum == expectedSum, success,failure);
				
				
				//Test the parsing methods
				std::vector<char *> args = {"test.cpp","-int","5","-double","-2.2","-schlom","hey","-boolio","1"};
				
				intArg.ListParse(args.size(),args.data());
				doubleArg.ListParse(args.size(), args.data());
				expectedSum = 5 - 2.2;
				sum = intArg + doubleArg;
				success = "List Parse methods successful";
				failure = "List Parse methods failed";
				basicConditionCheck(sum == expectedSum, success,failure);
				
				bool boolsuccess = true;
				try
				{
					JSL::Argument<bool> boolArg(false,"boolio",args.size(),args.data());
					boolsuccess = boolArg;
				}
				catch (...)
				{
					boolsuccess = false;
				}
				success = "Boolean parsing worked";
				failure = "Boolean parsing failed";
				basicConditionCheck(boolsuccess,success,failure);
				
				bool stringsuccess = true;
				try
				{
					JSL::Argument<std::string> stringArg("ho","schlom",args.size(),args.data());
					stringsuccess = (std::string)stringArg == "hey";
				}
				catch (...)
				{
					stringsuccess = false;
				}
				success = "String parsing worked";
				failure = "String parsing failed";
				basicConditionCheck(stringsuccess,success,failure);
				
				bool intAvert = true;
				try
				{
					JSL::Argument<int> intArg(2,"double",args.size(),args.data());
					intAvert = false;
				}
				catch (...)
				{
					intAvert = true;
				}
				success = "Successfully threw an error when casting an int to a double";
				failure = "Failed to throw an error when setting an int to a double....";
				basicConditionCheck(intAvert,success,failure);
			}
	};
	
	class MathTest : public JSL::UnitTest
	{
		public:
			MathTest()
			{
				Name = "JSL::Math";
				NonCriticalFailure = false;
				MessageBuffer.resize(0);
				BufferedTest();
			}
			void Run_Test()
			{
				VectorTest();
			}
			void VectorTest()
			{
				Passed = true;
				
				const int n = 5;
				JSL::Vector v1(n);
				
				bool allEqualToZero = true;
				for (int i = 0; i < n; ++i)
				{
					if (v1[i] != 0)
					{
						allEqualToZero = false;
					}
				}
				std::string message = "JSL::Vector Zero-initialisation method and basic access test";
				basicConditionCheck(allEqualToZero,message);
				
				
				
				v1[0] = n;
				bool basicAssignmentCheck = (v1[0] == n);
				message = "JSL::Vector Basic array-assignment syntax";
				basicConditionCheck(basicAssignmentCheck,message);
				
				
				std::vector<double> initStruct(n,0);
				initStruct[0] = n;
				JSL::Vector v2(initStruct);
				bool v2Init = true;
				for (int i = 0; i < n; ++i)
				{
					if (v2[i] != v1[i])
					{
						v2Init = false;
					}
				}
				message = "JSL::Vector initialisation using std::vector";
				basicConditionCheck(v2Init,message);
				
				bool overFlowSuccess;
				try
				{
					double a = v1[n+1];
					overFlowSuccess = false;
				}
				catch (std::runtime_error)
				{
					overFlowSuccess = true;
				}
				message = "JSL::Vector overflow error-throwing";
				basicConditionCheck(overFlowSuccess,message);
				
				bool underFlowSuccess;
				try
				{
					double a = v1[-1];
					underFlowSuccess = false;
				}
				catch (std::runtime_error)
				{
					underFlowSuccess = true;
				}
				message = "JSL::Vector underflow error-throwing";
				basicConditionCheck(overFlowSuccess,message);
				
				bool equalityCheck = (v1 == v2);
				message = "JSL::Vector equality testing";
				basicConditionCheck(equalityCheck,message);
				
				++v2[0];
				bool inEqualityCheck = (v1 != v2);
				message = "JSL::Vector inequality testing";
				basicConditionCheck(inEqualityCheck,message);
				
				
				double offset = 3.2;
				JSL::Vector v3 = v1 + v2;
				JSL::Vector v4 = offset + v1;
				JSL::Vector v5 = v1 + offset;
				bool summationWorks = true;
				for (int i = 0; i < n; ++i)
				{
					bool summer = (v3[i] == (v1[i] + v2[i]));
					bool scalarsum = (v4[i] == v1[i] + offset);
					bool sumscalar = (v5[i] == v1[i] + offset);
					
					if (!summer || !scalarsum || !sumscalar)
					{
						summationWorks = false;
					}
				}
				message = "JSL::Vector summation testing";
				basicConditionCheck(summationWorks,message);
				
				double mult = 3;
				bool scalarMultiplicationWorks = true;
				v1[0] = 4;
				JSL::Vector v6 = mult * v1;
				JSL::Vector v7 = v1 * -mult;
				for (int i =0; i < n; ++i)
				{
					bool forward = v6[i] == mult * v1[i];
					bool backward = v7[i] == -mult * v1[i];
					if (!forward || !backward)
					{
						scalarMultiplicationWorks = false;
					}
				}
				message = "JSL::Vector scalar multiplication testing";
				basicConditionCheck(scalarMultiplicationWorks,message);
				
				
				JSL::Vector v8 = v6 - v7;
				JSL::Vector v9 = v7 - v6;
				bool subtractCheck = (v8 == 2*mult*v1) && (v9 == -2*mult*v1);
				message = "JSL::Vector subtraction testing";
				basicConditionCheck(subtractCheck,message);
				
				
				bool divisorCheck = 0.5 * v1 == v1/2;
				try
				{
					JSL::Vector v10 = v1/0;
					divisorCheck = false;
				}
				catch (std::runtime_error)
				{
					//
				}
				
				message = "JSL::Vector division testing (including zero division)";
				basicConditionCheck(divisorCheck,message);
				
				
				v1[n-1] +=3;
				v2[n-1] += 9;
				double expectedDot = 0;
				double expectedNorm = 0;
				for (int i = 0; i < n;++i)
				{
					expectedDot += v1[i] * v2[i];
					expectedNorm += v1[i] * v1[i];
				}
				bool dotProductWorks = VectorDotProduct(v1,v2) == expectedDot && v1.Dot(v2) == expectedDot;
				message = "JSL::Vector Dot() member and VectorDotProduct function test";
				basicConditionCheck(dotProductWorks,message);
				
				bool normWorks = sqrt(expectedNorm) == v1.Norm();
				message = "JSL::Vector Norm() and SqNorm() member test";
				basicConditionCheck(normWorks, message);
				
				// angle & cross product checks 
				JSL::Vector u1({3,-2,5});
				JSL::Vector u2({8,7,2});
				JSL::Vector expectedCross({-39,34,37});
				double expectedAngle = 1.266158693;
				double recoveredAngle = u1.AngleBetween(u2);
				bool angleCorrect = abs(recoveredAngle - expectedAngle) < 1e-8;
				message = "JSL::Vector AngleBetween() and AngleBetweenVectors() test";
				basicConditionCheck(angleCorrect, message);
				
				message = "JSL::Vector Cross() and VectorCrossProduct() test";
				basicConditionCheck(u1.Cross(u2) == expectedCross,message);
			}
	
	};
	
	void RunAllTests()
	{
		std::cout << "Beginning testing of all JSL objects...\n" << std::endl;
		
		//The ordering of these tests is important! Do not shuffle them around.
		MetaTest metaT;
		metaT.Results();
		
		StringTest stringT;
		stringT.Results();
		
		IOTest ioT;
		ioT.Results();
		
		ArgumentTest argT;
		argT.Results();
		
		MathTest mathT;
		mathT.Results();
		
		std::cout << "\nTesting complete." << std::endl;
		
	}
}	
	
