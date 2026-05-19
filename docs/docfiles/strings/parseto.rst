.. _parseto:

String Parsing
====================

The inverse problem to :ref:`representing a type as a string <makestring>` is *parsing*, converting an input string representing a value into a typed-instance containing that value. 


``ParseTo<T>()``
-----------------

.. cpp:function:: template<class T> T ParseTo(std::string_view sv)
	
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

	Custom types can be fitted into the ``ParseTo`` paradigm either by adding a constructor which accepts a ``string_view`` as an argument.

.. dropdown:: Template Overloads
	:color: primary
	:icon: plug

Nested Parsing
---------------------

Parsing of objects of the form ``vector<set<T>>`` or nested structures is fully supported, provided that all inner types are individually supported by the engine. Due to the structural tokenization required to parse nested hierarchies, the input string must adhere to the following layout rules:

#. **Supported Endcaps:** Elements can be grouped using ``()``, ``[]``, or ``{}``. These delimiters must match perfectly; asymmetric pairings (e.g., ``(a,b}``) will trigger a runtime parsing failure.
#. **Optional Outer Envelopes:** Top-level containers do not require an outermost wrapper. Both explicitly enveloped structures (e.g., ``[[a,b],[c,d]]``) and raw sibling sequences (e.g., ``[a,b],[c,d]``) are supported out-of-the-box.
#. **Delimiter Propagation:** Outer layers are tokenized purely by evaluating balanced bracket boundaries. Any characters residing *outside* of a balanced bracket pair in an outer layer are skipped. Consequently, expressions like ``[[a,b] @ [c,d]]`` and ``[[a,b],[c,d]]`` yield identical results regardless of the provided delimiter sequence. Tokenization of the innermost values falls back to the specified ``delimiter`` sequence. For example:
   
   .. code-block:: cpp

      // Passes the "-" delimiter directly down to the innermost integers
      auto result = ParseTo<std::vector<std::vector<int>>>("[ [1-2-3], [4-5-6] ]", "-");

Usage
------------