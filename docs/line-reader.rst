.. line-reader

#######################
File Line Reader
#######################


Iterating through an input file is something I find myself doing in almost every project I write. It is incredibly boilerplate code, but can be difficult to generalise as you wish to execute custom code within a loop. 

The two file line reader codes here are therefore a set of macros which generalise this boilerplate code and make it simple to execute such boilerplate loops.


Warning!
*******************************

**Be careful when using these commands**

They are somewhat hacky macros which wrap around custom code. This means you need to be exceptionally careful:

* Compilation error messages can become garbled and confusing if arising from code piped into the macro. Read through the error logs carefully to isolate the problem. 
* The following variable names will temporarily go out of scope as they are re-utilised by the code (they re-enter scope when the block ends - note that this means that you do not need to worry about the macros overwriting your other variables, which is some comfort):

	* ``std::string FILE_LINE``
	* ``std::vector<std::string> FILE_LINE_VECTOR``

* Use parentheses (), not curly braces {} around the code block.

* After the close-paren, remember a semi-colon.

* They are not within the JSL namespace, annoyingly

Macros
************

.. doxygendefine:: forLineIn

.. doxygendefine:: forLineVectorIn

Example Usage
*********************

If the file ``sonnet116.txt`` contains the full text of `the relevant Shakespearean Sonnet <https://www.poetryfoundation.org/poems/45106/sonnet-116-let-me-not-to-the-marriage-of-true-minds>`_, and we wished to find the 4th word of any line more than 40 characters long:

.. code-block:: c++
	:emphasize-lines: 1
	
	//sonnet.cpp
	
	#include <iostream>
	#include "JSL.h"
	int main(int argc, char * argv[])
	{
	 std::string fileName = "sonnet116.txt";
	 forLineVectorIn(fileName,' ',
	   if (FILE_LINE.length() > 40)
	   {
		  std::cout << FILE_LINE_VECTOR[3] << std::endl;
	   }
	 );
	}


When run:

.. code-block:: text

	>./sonnet
	tempests
	although
	fool,
	sickle's
	with
	out

Note that because we defined "words" to be delineated only by spaces, we retained the comma after "fool". We could add further code into the macro to remove this, if we desired.
