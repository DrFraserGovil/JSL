.. _makestring:

String Representation
=======================


.. cpp:function:: template<class T> std::string JSL::String::makeFrom(const T & obj) 

	A generic interface to convert input objects into strings for serialisation or display

	:tparam T: An arbitrary type  
	:param obj: The value to be stringified
	:return: A string representing the input value
	:throws template_errors: If std::string is not constructible from the object, and no specific overload exists 


.. dropdown:: Template Overloads
	:color: primary
	:icon: plug

	The default template function only works if the template parameter has an existing string-constructor; which somewhat defeats the point of writing a unified interface. To that end, we provide a number of template overloads which implement string-conversion. 

	:Numeric Types:
	
	.. cpp:function:: template<JSL::Concept::Numeric T> std::string JSL::String::makeFrom(const T & obj) 

		Converts any numeric type into a string 

		:details: Uses the principle of 'least representation' to determine length of output string. For more control over the output format, see :ref:`precision`.
		:tparam T: A numeric type (except bool and char)
		:param obj: The value to be stringified
		:return: A string representing the input value
		:throws std::runtime_error: If the value cannot be converted to a string


	:Booleans:

	.. doxygenfunction:: JSL::String::makeFrom(const bool & obj)

	:Containers:


	.. cpp:function:: template<JSL::Concept::NonStringRange T> std::string JSL::String::makeFrom(const T & obj) 

		Converts an iterable range into a string, recursively converting the contained objects

		:details: The string has bracket endcaps ("[]") and a comma delimiter. For more fine grained control, see JSL::String::stitch
		:tparam T: An type supporting range-based iteration (but not strings)
		:param obj: The value to be stringified
		:return: A string representing the input value
		:throws std::runtime_error: If the value cannot be converted to a string

	.. cpp:function:: template<JSL::Concept::TupleLike T> std::string JSL::String::makeFrom(const T & obj) 

		Converts an heterogenous tuple-like (including std::pair) into a string, recursively converting the contained objects

		:details: The string has bracket endcaps ("()") and a comma delimiter. 
		:tparam T: Any std::tuple or std::pair object 
		:param obj: The value to be stringified
		:return: A string representing the input value
		:throws std::runtime_error: If the value cannot be converted to a string
	:Characters:

	C-style strings ``char *`` are already handled by the generic interface, but individual ``char`` objects are not constructible into strings, and so need a special overload:

	.. doxygenfunction:: JSL::String::makeFrom(const char & obj)

.. _precision:

Changing Precision
---------------------


.. cpp:function:: template<std::floating_point T> std::string JSL::String::makeFrom(const T& obj, int precision, std::chars_format fmt = std::chars_format::general)

	Converts floating-point types into a string with a specified format.

	:tparam T: A floating-point type.
	:param obj: The value to be stringified.
	:param precision: The number of decimal places to retain.
	:param fmt: The format to be used (either fixed or scientific).
	:return: A string representing the input value.

.. cpp:function:: template<JSL::Concept::Integral T> std::string JSL::String::makeFrom(const T &obj, int precision, std::chars_format fmt = std::chars_format::general)

	A specialised overload for large integers

	:details: Internally, casts input to a double, and then hands to the floating_point implementation. This has the following consequences:
	
		* If ``fmt = std::chars_format::general``:
  			#. ``precision > log10(value)``, the integer is rendered exactly in decimal notation
			#. Otherwise, rendered as a rounded scientific notation to the specified precision
		* If ``fmt = std::chars_format::fixed`` , a number of zeros equal to ``precision`` are appended to the end of the (exactly rendered) integer
		* If ``fmt = std::chars_format::scientific``, converts to scientific notation and treated as a double with the specified precision 