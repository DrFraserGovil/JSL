.. _vectorjoin:

Vector Joining
==================

Functions
--------------

Appending and joining vectors is something that the existing STL does support, but in order to be powerful and general, it is also verbose. The functions here provide less powerful, but more succinct operations for 'just sticking two vectors together'.

.. jsl-meta:: 
	:file: Vectors/Join.h
	:namespace: JSL::Vector

.. doxygenfunction:: JSL::Vector::append
.. doxygenfunction:: JSL::Vector::prepend
.. doxygenfunction:: JSL::Vector::concat

Usage 
----------

.. code-block:: cpp
	
	#define JSL_INCLUDE_LOG
	#include <JSL.h>
	#include <thread>
	#include <chrono>
	#include <cmath>

	void print(std::string msg, std::vector<int> & a)
	{
		std::cout << msg <<" " << JSL::String::represent(a) << std::endl;
	}
	int main(int argc,char**argv)
	{
		std::vector<int> a{1,2,3};
		std::vector<int> b{4,5,6};
		std::vector<int> c{7,8,9};

		print("Original: ", a);
		
		JSL::Vector::append(a,b);
		print("Appended: ",a);

		JSL::Vector::prepend(b,c);
		print("Prepended:",b);

		auto ac = JSL::Vector::concat(a,c);
		print("Concat:   ",ac);
	}

Output
#########

.. code-block:: shell-session
	
	Original:  [1, 2, 3]
	Appended:  [1, 2, 3, 4, 5, 6]
	Prepended: [7, 8, 9, 4, 5, 6]
	Concat:    [1, 2, 3, 4, 5, 6, 7, 8, 9]