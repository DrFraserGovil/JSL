.. _wrapping:

Wrapping Functions
===================

These are the functions which underly the basic string-wrapping utilities in the library

.. jsl:: JSL::String::wrap
	:file: Strings/FoldLine.h
	:command: doxygenfunction

.. doxygenfunction:: JSL::String::wrapToString


Usage
/////////////

.. code-block:: cpp
	
	int main(int argc,char**argv)
	{
		JSL::Log::Global().ShowHeaders = false;
		LOG(INFO) << "Your terminal has: ";
		LOG(INFO) << "\tColumns: " << JSL::Terminal::Global().Columns();
		LOG(INFO) << "\tTabsize: " << JSL::Terminal::Global().TabSize();

		std::string stringA = "This is a long string,\tlonger than\tthe size of the terminal\nWe can manually linebreak\n\tAnd have indented text";
		size_t w = JSL::Terminal::Global().Columns();    
		auto l = JSL::String::wrap(stringA,w-1);///to make room for our eol-character
		
		for (auto & line : l)
		{
			std::cout  << line << "|\n";
		}

		//equivalently
		// std::cout << JSL::String::wrapToString(stringA,w-1,"|\n") << "|" <<std::endl;
	}

Giving output (on a terminal we set the sizes for, for the purposes of this demonstration)

.. code-block:: shell-session
	
	> ./test
	Your terminal has:
	   Columns: 50
	   Tabsize: 3
	This is a long string,  longer than the size of  |
	the terminal                                     |
	We can manually linebreak                        |
	   And have indented text                        |

Note that the first tab block was correctly calculated as only 2-charaters wide, and the second (which straddled the wrapping break) is absorbed into the break. The tab which occured at the beginning of a manual linebreak, however, is preserved. 

We can run the same code after manually altering the tab size:

.. code-block:: shell-session

	> tabs -7	
	> ./test
	Your terminal has:
		   Columns: 50
		   Tabsize: 7
	This is a long string,      longer than   the    |
	size of the terminal                             |
	We can manually linebreak                        |
	       And have indented text                    |