.. pipes

###################
Piped Input Manager
###################

When passing information along a daisy chain of different codes, linux users are other prone to using the powerful pipe command:

.. code-block:: bash

	grep "words_containing_pattern" | wc -l

This command returns the number of words containing the specified pattern, by first finding them (grep), and then counting the number of lines in the output from the grep, passed along by the \|. Equally, one can redirect a file as input to a program:

.. code-block:: bash

	./test < text.txt

In both cases, the shell takes the input (the output of a command in the case of the pipe, and the contents of a file in the case of a redirect), and puts it into the *standard input channel* (stdin) for the program which then runs. For simplicity, we term both cases "piped input", regardless of whether it arrived via pipes or redirects.

The commands below provide a simple way to interact with that input.

Pipe Checker
************

Before running any of the following commands, you must first determine if the standard input is the default (i.e. user input), or being piped to.

.. doxygenfunction:: JSL::PipedInputFound

Basic Pipe Read
****************

This is the most basic pipe funcion -- it reads the input in line by line, and saves it to a string (including line break characters), which is then returned. In general, this usage is discouraged since, in most cases, you would then need to go and parse the string in a separate function, in which case the Pipe Macros below would cut out the middle man.

.. doxygenfunction:: JSL::readPipedInputString

Pipe Macros
**************

Similar in many ways to the :ref:`file reader macros<File Line Reader>` macros, these are hacky functional macro definitions, which allow the insertion of arbitrary code inside the pipe-reading. These functionals temporarily take control of the variables PIPE_LINE and PIPE_LINE_VECTOR (as with the file reader macros), and permit interaction with them.


Basic Macro
----------------
The first macro simply loops over the input lines, and saves them individually to the PIPE_LINE variable, where they can be parsed by other code and saved to other variables.

.. doxygendefine:: forLineInPipedInput

Vector Macro
--------------

The vector macro functions identically to the basic macro, with the addition that it calls :ref:`split()<Split Function>` on the input, with the value of the chosen token, thereby breaking the line into tokens, stored in the std::vector<std::string> object PIPE_LINE_VECTOR. 

.. doxygendefine:: forLineVectorInPipedInput


Example Usage
----------------

Consider the following code

.. code-block:: c++

	:emphasize-lines: 1
	
	//toggle.cpp
	#include <iostream>
	#include "JSL.h"

	int main(int argc, char * argv[])
   	{
	   bool selfReferentialToggle = false;
		forLineInPipedInput(
			bool containsToggle = PIPE_LINE.find("selfReferentialToggle = true") != std::string::npos;
			bool isIfCondition = (PIPE_LINE.find("containsToggle") != std::string::npos);
			if (containsToggle && !isIfCondition)
			{
				selfReferentialToggle = true;
				std::cout << "I found myself! " << PIPE_LINE << std::endl
			}
		);
	return 0;
	}

This code identifies if the source file it is pointed towards contains the selfReferentialToggle=true condition (and if so, sets seldReferentialToggle = true). An additional check was needed since the check to identify the trigger string necessarily also included the trigger string, and hence had to be removed.  The output is:

.. code-block:: bash

	$ cat toggle.cpp | ./toggle
	I have found myself!                    selfReferentialToggle = true;

If, however, we wanted to identify the line on which this occured, we could use:

.. code-block:: c++

	:emphasize-lines: 1
	
	//toggle2.cpp
	#include <iostream>
	#include "JSL.h"

	int main(int argc, char * argv[])
   	{
	   bool selfReferentialToggle = false;
		forLineVectorInPipedInput(
			bool containsToggle = PIPE_LINE.find("selfReferentialToggle = true") != std::string::npos;
			bool isIfCondition = (PIPE_LINE.find("containsToggle") != std::string::npos);
			if (containsToggle && !isIfCondition)
			{
				selfReferentialToggle = true;
				int line = stoi(PIPE_LINE_VECTOR[1]);
				std::cout << "I found myself on line " << line << "of file " << PIPE_LINE_VECTOR[0] << std::endl
			}
		);
		return 0;
	}

Then:

.. code-block:: bash

	$ grep -rn "=" | ./toggle2
	I have found myself on line 12 of file /path/to/toggle.cpp

