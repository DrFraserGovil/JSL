.. argument

#################
Argument Class
#################


Class Definition
*******************

.. doxygenclass:: JSL::Argument
	:members:
	:private-members:

Example Usage
******************

Consider the following code

.. code-block:: c++
	
	#include <iostream>
	#include "JSL.h"
   int main(int argc, char * argv[])
   {
	
	//Explicit initialisation + parsing
	
	JSL::Argument<int> integer1(1,"integer1");
	integer1.ListParse(argc,argv);
	
	//Parsing at construction time
	JSL::Argument<int> integer2(5,"integer2",argc,argv);
	
	
	//Parsing from a configuration file
	std::string configFile = "config.txt";
	char delimiter = ",";
	JSL::Argument<double> double1(2.5,"double1",configFile, delimiter);
	JSL::Argument<std::string> string1("Goodbye","welcome-string",configFile,delimiter)
	
	
	//Print the output & Demonstrate the implicit casting ability of the "()" to work on almost everything except strings
	std::cout << (std::string)string1 << "\n";   // <- have to explicitly cast here because of char/string shenanigans. Everything else works.
	std::cout << integer1 << "\n";
	std::cout << integer2 << "\n";
	std::cout << double1 << "\n";
	
	std::cout << integer1 + integer2 << "\n";
	std::cout << integer1 + integer2 - double1 << "\n";
   }

When called in the same directory as the configuration file `config.txt`

.. code-block:: c++
	:emphasize-lines: 1
	
	config.txt
	
	integer1, 3
	integer2, 23
	double1, 101.1
	welcome-string, Hello World!
	
The output is::

	> ./argument 
	Hello World!
	integer1 = 1
	integer2 = 5
	double1 = 101.1
	integer1 + integer2 = 6
	(integer2 - integer1) * double1 = 404.4
	
	>./argument -integer1 3 -integer2 29 -double1 2.2
	Hello World!
	integer1 = 3
	integer2 = 29
	double1 = 101.1
	integer1 + integer2 = 32
	(integer2 - integer1) * double1 = 2628.6


The wrapper class ArgumentInterface also allows us to build heterogenous arrays which we can loop over to allow for easy & generalised initialisation:

.. code-block:: c++
	
	JSL::Argument<int> integer1(1,"integer1");
	JSL::Argument<int> integer2(2,"integer2");
	JSL::Argument<double> double1(1.5,"double1");
	JSL::Argument<double> double2(-2.2,"double2");
	JSL::Argument<bool> bool1(true,"bool1");
	
	std::vector<JSL::ArgumentInterface *> arguments = {&integer1, &integer2, &double1, &double2, &bool1};
	for (int i = 0; i < arguments.size(); ++i)
	{
		arguments[i]->ListParse(argc,argv);
	}

Adding additional parameters merely requires initialising them and adding them to the `arguments` list. 

Oddities & Notes of Caution
****************************************

In the first example, note that although the `-double1 2.2` pair was passed, the code still used the default value `double1 = 101.1`, as this parameter was only ever assigned to use the configuration file, rather than the command line agruments. 

There is no centralised list of assigned TriggerStrings, so it is entirely possible for two different parameters to have the same name. Both will initialise to the same value. Equally, there is no checking if a parameter is assigned to multiple times (either from repeated arguments, or from both the command line and a configuration file). The command line/configuration files are read sequentially from top to bottom.


