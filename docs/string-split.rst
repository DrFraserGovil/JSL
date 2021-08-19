.. string-split

#######################
Split Function
#######################

.. doxygenfunction:: JSL::split


Example Usage
********************

Basic usage:

.. code-block:: c++
	:emphasize-lines: 1
	
	//fox.cpp
	
	#include <iostream>
	#include <vector>
	#include "JSL.h"
	int main(int argc, char * argv[])
	{
	  //Simple word splitting
	  std::string text = "The quick brown fox jumped over the lazy dog";
	  std::vector<std::string> words = JSL::split(text,' ');
	  for (auto word : words)
	  {
	    std::cout << word << " || ";
	  }
	}
	
Giving::
	
	>./fox
	The || quick || brown || fox || jumped || over || the || lazy || dog || 
	

Double tokenization:

.. code-block:: c++
	:emphasize-lines: 1
	
	//goo.cpp
	
	#include <iostream>
	#include <vector>
	#include "JSL.h"
	int main(int argc, char * argv[])
	{
	  //Double token omission
	  std::string text = "Boo! The grey goo came oozing into the room, around the crooked books" ;
	  std::vector<std::string> words = JSL::split(text,' ');
	  for (auto word : words)
	  {
	    std::cout << word << " || ";
	  }
	}
	
Giving::

	>./goo
	B||! The grey g|| came ||zing int|| the r||m, ar||und the cr||ked b||ks||

Note that under a naive tokenization scheme, ``Boo!`` would be split into 3 elements: ``{"B","","!"}``, and rendered as ``B||||!``. However, we omit any empty elements. 
