.. _parseto:

String Parsing
====================

The inverse problem to representing a type as a string is *parsing*, converting an input string representing a value into a typed-instance containing that value. 

.. warning::
	Whilst it is guaranteed that all types which can be :ref:`parsed-to <parseto>` can be made-from, **the inverse is not true**, due to the support of a ``stringstream`` converter.

	In order to enable ``ParseTo<CustomType>``,  
	
``ParseTo()``
---------------



.. dropdown:: Template Overloads
	:color: primary
	:icon: plug


Usage
------------