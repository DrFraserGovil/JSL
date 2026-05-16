.. _tabularwrap:

Tabular Formatting
======================
Functions
---------------


.. doxygenfunction:: JSL::String::tableFormat(std::vector<std::string_view> input, std::vector<size_t> widths, std::string_view delimiter,std::string_view endCap)
.. doxygenfunction:: JSL::String::tableFormat(std::vector<std::string_view> input, size_t width, std::string_view delimiter,std::string_view endCap)

Usage
----------

.. code-block:: cpp

	int main(int argc,char**argv)
	{
		JSL::Log::Global().ShowHeaders = false;
		LOG(INFO) << "Your terminal has: ";
		LOG(INFO) << "\tColumns: " << JSL::Terminal::Global().Columns();
		LOG(INFO) << "\tTabsize: " << JSL::Terminal::Global().TabSize();

		std::string stringA = "This is a long string,\tlonger than\tthe size of the terminal\nWe can manually linebreak\n\tAnd have indented text";
		std::string stringB = "This is a short string";
		std::string stringC = "This is a medium length string, to show that stringB is still being properly padded";
		size_t w = JSL::Terminal::Global().Columns();    
		size_t c1 = w*0.4;
		size_t c2 = w*0.3;
		size_t c3 = w - (c1 + c2); 

		std::cout << JSL::String::tableFormat({stringA,stringB,stringC},{c1,c2,c3},"| ","|)");
		//ruler to show the delims/end caps are accounted for
		std::cout << std::string(w-1,'-') << "^" << std::endl; 
	}

Output

.. code-block:: shell-session

	Your terminal has:
	    Columns: 50
	    Tabsize: 4
	This is a    | This is a short string  | This is a|)
	long string, |                         | medium   |)
	longer than  |                         | length   |)
	the size of  |                         | string,  |)
	the terminal |                         | to show  |)
	We can       |                         | that     |)
	manually     |                         | stringB  |)
	linebreak    |                         | is still |)
	    And have |                         | being    |)
	indented text|                         | properly |)
	             |                         | padded   |)
	---------------------------------------------------^