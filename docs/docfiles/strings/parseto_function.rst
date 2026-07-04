.. _parsetofunc:

The ``ParseTo<T>()`` Template 
===============================

.. jsl-meta::
	:namespace: JSL::String
	:file: Strings/ParseTo.h

.. cpp:function:: template<class T> T String::ParseTo(std::string_view sv)
	
	The default template for parsing a string into a generic type

	Attempts a naive string-native construction. We don't require that such a construction exists as this is also our gateway to error messages

	:tparam T: A type which can be converted into a string 
	:param sv: A string to be parsed 
	:return: An object of type T represented by the input string

Supported Types
-----------------

.. warning::
	We guarantee that any inbuilt type supported by ``ParseTo`` is supported by ``makeFrom``, but the inverse is not true due to serialisation via the stream operator

The following types have overloads and support in the :ref:`makestring` module to support full round-trip serialisation:


* All string-like types (``string``, ``string_view``, ``char *``, ``char``) and objects which can implicitly cast-to-and-from strings (i.e. ``std::filesystem::path``)
* All numeric types (``int``, ``short``, ``long``, ``double``, ``unsigned int`` and so on)
* Booleans (as ``true/false``) 
* All iterable containers (``vector``, ``set`` etc., provided their inner types are supported)
* Tuple-likes (including ``pair``, provided their inner types are supported)
* Nullable wrappers (``std::optional``, ``std::shared_ptr`` and ``std::unique_ptr`` wrappers of all supported types. 
* Time durations of the form ``std::chrono::duration<T>`` 
* Recursive nesting of containers (i.e. tuples-of-vectors-of-pairs), provided that all types in the stack are supported, and the compiler's recursion depth isn't hit. See below for more details.


.. note:: 

	Custom types can be fitted into the ``ParseTo`` paradigm by adding a custom constructor which accepts a ``string_view`` as an argument.