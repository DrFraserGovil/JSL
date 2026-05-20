.. _parseto:

Parsing Strings
====================

The inverse problem to :ref:`representing a type as a string <makestring>` is *parsing*, converting an input string representing a value into a typed-instance containing that value. 

.. toctree::
	parseto_function
	parseto_helpers
	parseto_nesting
	:maxdepth: 1

Usage
------------

.. code-block:: cpp
	
	#include <JSL.h>

	template<class T>
	void runTest(std::string_view target,std::string_view niceName)
	{
		std::cout << "<" << niceName << ">" << std::string(std::max(1,(int)(10-niceName.size())),' ') << "('" << target << "') = ";
		try
		{
			T tmp = JSL::String::ParseTo<T>(target);
			std::cout  << JSL::String::makeFrom(tmp) << std::endl;
		}
		catch (const std::exception & e)
		{
			std::cout << "ERROR: " << e.what() << std::endl;
		}    
	}

	int main(int argc,char**argv)
	{
		std::cout << "NUMERICS\n-------------\n";
		std::vector<std::string> numtests = {"0","1","18.76","-1e13"};

		for (auto & test : numtests)
		{
			runTest<int>(test,"int");
			runTest<double>(test,"double");
			runTest<bool>(test,"bool");
		}
		std::cout << "\nSTRINGS\n-------------\n";
		std::vector<std::string> stringtests = {"a","hello world"};
		for (auto & test : stringtests)
		{
			runTest<char>(test,"char");
			runTest<std::string>(test,"string");
		} 
		std::cout << "\nCONTAINERS\n-------------\n";
		std::vector<std::string> containerTests = {"[1,2.2] (0,4)"};
		for (auto & test : containerTests)
		{
			runTest<std::vector<std::pair<bool,double>>>(test,"vec<pair<>>");
			using floatMicro = std::chrono::duration<double, std::ratio<1,1000000>>;
			runTest<std::vector<std::vector<floatMicro>>>(test,"vec<vec<>>");
		}
		
		std::cout << "\nOPTIONAL & POINTERS\n--------------\n";
		std::vector<std::string> nullTests = {"", JSL_NULL_STRING, "10","1e17"};
		for (auto & test : nullTests)
		{
			runTest<std::optional<double>>(test,"opt-double");
			runTest<std::unique_ptr<int>>(test,"unique-int");
		}
	}

Giving output:

.. code-block:: shell-session

	NUMERICS
	-------------
	<int>       ('0') = 0
	<double>    ('0') = 0
	<bool>      ('0') = false
	<int>       ('1') = 1
	<double>    ('1') = 1
	<bool>      ('1') = true
	<int>       ('18.76') = ERROR: Could not complete conversion    Partial conversion of `18.76` to type i unconverted characters were: .76
	<double>    ('18.76') = 18.76
	<bool>      ('18.76') = ERROR: Cannot complete string-boolean conversion    Cannot convert string 18.76 to boolean
	<int>       ('-1e13') = ERROR: Could not complete conversion    Partial conversion of `-1e13` to type i unconverted characters were: e13
	<double>    ('-1e13') = -1e+13
	<bool>      ('-1e13') = ERROR: Cannot complete string-boolean conversion    Cannot convert string -1e13 to boolean

	STRINGS
	-------------
	<char>      ('a') = a
	<string>    ('a') = a
	<char>      ('hello world') = ERROR: Cannot complete string-char conversion Cannot convert string_view 'hello world' to char: Expected a single character.
	<string>    ('hello world') = hello world

	CONTAINERS
	-------------
	<vec<pair<>>> ('[1,2.2] (0,4)') = [(true, 2.2), (false, 4)]
	<vec<vec<>>> ('[1,2.2] (0,4)') = [[1us, 2.2us], [0us, 4us]]

	OPTIONAL & POINTERS
	--------------
	<opt-double> ('') = -null-
	<unique-int> ('') = -null-
	<opt-double> ('-null-') = -null-
	<unique-int> ('-null-') = -null-
	<opt-double> ('10') = 10
	<unique-int> ('10') = 10
	<opt-double> ('1e17') = 1e+17
	<unique-int> ('1e17') = ERROR: Could not complete conversion    Partial conversion of `1e17` to type i unconverted characters were: e17