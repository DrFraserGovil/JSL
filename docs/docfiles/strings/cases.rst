.. _stringcase:

Strings & Cases
=================


Equality checks
--------------------

.. jsl:: JSL::String::iEquals
	:file: Strings/Cases.h
	:command: doxygenfunction

In-Place Conversion
----------------------

These functions mutate the input string in-place to obey the new case rules

.. jsl-meta::
	:file: Strings/Cases.h
	:namespace: JSL::String	

.. doxygenfunction:: JSL::String::toUpper
.. doxygenfunction:: JSL::String::toLower
   
Copy Conversion
-------------------
   
These functions return a copy of the original string, which has been mutated. The original string is unchanged.

.. doxygenfunction:: JSL::String::getUpper
.. doxygenfunction:: JSL::String::getLower