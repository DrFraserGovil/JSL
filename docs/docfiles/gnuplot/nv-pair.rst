.. nv-pair

###########################
Specifier Name-Value Pairs
###########################


Name Value Pairs are used in order to allow arbitrary data specification into the :ref:`Plot Data Constructor<Plot Data>`. A Name Value pair object is exactly as it sounds -- a struct containing a name, and an arbitrary data type associated with that name. 


Usage
-------------------

Name-Value Pairs can either be constructed explicitly


.. code-block:: c++
	
	#include <iostream>
	#include "JSL.h"
	int main(int argc, char * argv[])
	{
	 	using namespace JSL;
		Vector x = Vector::linspace(0,10,100)
		auto y = x + 10;

		gnuplot gp;
		gp.Plot(x,y,NameValuePair(PenSize,10),NameValuePair(Colour,"red"));
		gp.Show();
	}

Or, they can be constructed by one of the constructors provided within the JSL::LineProperty namespace:

.. code-block:: c++
	
	#include <iostream>
	#include "JSL.h"
	int main(int argc, char * argv[])
	{
	 	using namespace JSL;
		Vector x = Vector::linspace(0,10,100)
		auto y = x + 10;

		gnuplot gp;
		gp.Plot(x,y,LineProperty::Colour("green"),LineProperty::Legend("Straight Line"));
		gp.Show();
	}

The full list of constructors is listed below.

Permitted Names
------------------

Whilst arbitrary names (i.e. strings) would be possible, we limit the names to members of the following enum group, and hence ensure that all NV-pairs are associated with a valid line property.

.. doxygenenum:: JSL::Property

NameValue Pair Object
----------------------------

.. doxygenstruct:: JSL::NameValuePair

LineProperty Functions
-------------------------

.. doxygenfunction:: JSL::LineProperties::Colour

.. doxygenfunction:: JSL::LineProperties::PenSize

.. doxygenfunction:: JSL::LineProperties::PenType

.. doxygenfunction:: JSL::LineProperties::ScatterType

.. doxygenfunction:: JSL::LineProperties::Legend