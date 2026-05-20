.. _makestring:

Representating Objects as Strings
===================================

There are many different methods to convert types into strings - from direct ``static_cast<std::string>`` casting, to ``to_string()`` function calls. The aim of this module is to provide a consistent interface for converting the most common types into strings that represent their value. 

	

``makeFrom()``
----------------------

.. cpp:function:: template<class T> std::string JSL::String::makeFrom(const T & obj) 

	A generic interface to convert input objects into strings for serialisation or display

	Attempts to pipe the input object into an ``ostringstream``, and convert this to a string. This is generally quite slow, so other overloads are provided, even for types which can be piped in this fashion.

	:tparam T: An arbitrary type  
	:param obj: The value to be stringified
	:return: A string representing the input value
	:throws template_errors: If std::string is not constructible from the object, and no specific overload exists 

Supported Types
-------------------

The following types have overloads and support in the :ref:`parseto` module to support full round-trip serialisation:

* All string-like types (``string``, ``string_view``, ``char *``, ``char``) and objects which can implicitly cast-to-and-from strings (i.e. ``std::filesystem::path``)
* All numeric types (``int``, ``short``, ``long``, ``double``, ``unsigned int`` and so on)
* Booleans (as ``true/false``) 
* All iterable containers (``vector``, ``set`` etc., provided their inner types are supported)
* Tuple-likes (including ``pair``, provided their inner types are supported)
* Nullable wrappers (``std::optional``, ``std::shared_ptr`` and ``std::unique_ptr`` wrappers of all supported types. 
* Time durations of the form ``std::chrono::duration<T>`` 
* Recursive nesting of containers (i.e. tuples-of-vectors-of-pairs), provided that all types in the stack are supported, and the compiler's recursion depth isn't hit

.. warning::
	The default template for ``makeFrom`` works for any type which can be passed into a ``stringstream`` via ``operator<<()``; but such objects are not guaranteed to support round-trip serialisation (i.e. they may not be able to be recovered via ``ParseTo``  )

.. note:: 

	Custom types can be fitted into the ``makeFrom`` paradigm either by adding an explicit casting operator, ``operator std::string()`` (most efficient), or a streaming operator ``operator<<()`` (easiest to write). To ensure full round-trip capability, ensure they can be constructed from this same output.

.. dropdown:: Template Overloads
	:color: primary
	:icon: plug

	:String Types:

	.. cpp:function:: template<JSL::Concept::StringType T> std::string JSL::String::makeFrom(const T & obj) 

		A wrapper for converting string-like objects into full std::strings. Supports any object which has a string-casting operator. 

		This is a largely superfluous function, as it turns strings into strings. But it enables generalisable template libraries (i.e. :ref:`parameters`) to have a consistent interface. 

		:tparam T: A string-like type (i.e. std::string, char \*, string_view)
		:param obj: The string to be stringified
		:return: A string of the string
		
	

	The ``char`` type does not meet the criteria for being a ``StringType``, as there does not exist a constructor ``std::string(char)``, and so we need a special overload:

	.. doxygenfunction:: JSL::String::makeFrom(const char & obj)

	:Numeric Types:
	
	.. cpp:function:: template<JSL::Concept::Numeric T> std::string JSL::String::makeFrom(const T & obj) 

		Converts any numeric type into a string 

		Uses the principle of 'least representation' to determine length of output string. For more control over the output format, see :ref:`precision`.
		
		:tparam T: A numeric type (except bool and char)
		:param obj: The value to be stringified
		:return: A string representing the input value
		:throws std::runtime_error: If the value cannot be converted to a string

	.. doxygenfunction:: JSL::String::makeFrom(const bool & obj)

	:Containers:


	.. cpp:function:: template<JSL::Concept::NonStringRange T> std::string JSL::String::makeFrom(const T & obj) 

		Converts an iterable range into a string, recursively converting the contained objects

		The string has bracket endcaps ("[]") and a comma delimiter. For more fine grained control, see :ref:`JSL::String::stitch <joinstring>`
		
		:tparam T: An type supporting range-based iteration (but not strings)
		:param obj: The value to be stringified
		:return: A string representing the input value
		:throws std::runtime_error: If the value cannot be converted to a string

	.. cpp:function:: template<JSL::Concept::TupleLike T> std::string JSL::String::makeFrom(const T & obj) 

		Converts an heterogenous tuple-like (including std::pair) into a string, recursively converting the contained objects

		The string has bracket endcaps ("()") and a comma delimiter. 
		
		:tparam T: Any std::tuple or std::pair object 
		:param obj: The value to be stringified
		:return: A string representing the input value
		:throws std::runtime_error: If the value cannot be converted to a string

	:Nullable Wrappers:


	.. cpp:function:: template<JSL::Concept::OptionalLike T> std::string JSL::String::makeFrom(const T & obj) 

		Converts an optional-value into a string, returning either the value (if it exists), or :ref:`nullstring`.	
		
		:tparam T: Any std::optional object 
		:param obj: The value to be stringified
		:return: A string representing the input value
		:throws std::runtime_error: If the internal type is not supported, or cannot be converted to a string
	
	.. cpp:function:: template<JSL::Concept::SmartPointer T> std::string JSL::String::makeFrom(const T & obj) 

		Converts the value held by a smart pointer a into a string, returning either the value (if it exists), or :ref:`nullstring`.	
		
		This stringifies the object pointed to by the pointer, not the pointer itself

		:tparam T: Any std::unique_ptr or std::shared_ptr object 
		:param obj: The pointer associated with the object to be stringified
		:return: A string representing the pointed-to value
		:throws std::runtime_error: If the internal type is not supported, or cannot be converted to a string
	


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

	A specialised overload for changing the output format of integers

	:details: Internally, casts input to a double, and then hands to the floating_point implementation. This has the following consequences:
	
		* If ``fmt = std::chars_format::general``:
  			#. ``precision > log10(value)``, the integer is rendered exactly in decimal notation
			#. Otherwise, rendered as a rounded scientific notation to the specified precision
		* If ``fmt = std::chars_format::fixed`` , a number of zeros equal to ``precision`` are appended to the end of the (exactly rendered) integer
		* If ``fmt = std::chars_format::scientific``, converts to scientific notation and treated as a double with the specified precision 
	:tparam T: An integral type.
	:param obj: The value to be stringified.
	:param precision: The number of decimal places to retain.
	:param fmt: The format to be used (either fixed or scientific).
 	:return: A string representing the input value.

