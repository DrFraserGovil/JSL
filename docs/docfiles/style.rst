.. _style:

Style Guidelines
===================

The following are the style guidelines that we have tried to adhere to in creating the JSL. 

Naming Conventions
---------------------

#. **Cases**. The following rules regarding cases will be followed:
	+------------------------+-------------------------------------------------------+
	| ``Casing Style``       | Applied To                                            |
	+========================+=======================================================+
	| ``camelCase``          | * Free variables                                      |
	|                        | * Free functions                                      |
	+------------------------+-------------------------------------------------------+
	| ``PascalCase``         | * Namespaces                                          |
	|                        | * Class, Struct, and Enum types                       |
	|                        | * Member functions and variables                      |
	+------------------------+-------------------------------------------------------+
	| ``snake_case``         | * Disambiguating free functions which have both a     |
	|                        |   ``std::string`` and ``std::string_view`` version    |
	+------------------------+-------------------------------------------------------+
	| ``SNAKE_CASE``         | * Preprocessor Macros                                 |
	|                        | * Global-scope Enums and bitmask flags                |
	+------------------------+-------------------------------------------------------+

#. **Verbosity & Disambiguation**
	* Long, descriptive names are preferred over short, oblique ones.
	* Abbreviations in names should be discouraged except where they have meaning outside the library.
	* Names should avoid duplicating parts of their namespace (i.e. ``Vector::joinVector`` should be discouraged; ``Vector::join`` is perfectly descriptive).

#. **Global Variables**
	* Global variables within the library are permitted (though discouraged), if guards such as a singleton pattern is employed.  
	* All global variables must be nested within a separate ``Global`` namespace to indicate their nature (i.e. the ``JSL::Log::Global`` houses the global log settings), or accessed through a Meyers singleton called ``Global`` within the relevant namespace.

Coding Style
--------------

#. **Modern C++ Standard**.
    * JSL actively targets the modern C++20 standard. Utilization of features therein (such as concepts, spans, and views) is highly encouraged.

#. **Memory & Pointer Ownership**.
    * Explicit memory management via raw ``new`` and ``delete`` operators is strictly prohibited.
    * Smart pointers (such as ``std::unique_ptr``) must be utilized for all heap allocations.
    * Raw pointers are permitted exclusively for non-owning observers referencing stack-allocated objects.

#. **Bracing Style**.
    * Braces associated with namespaces, classes, functions, and control blocks must always reside on their own line (Allman style).
    * An exception is permitted for extremely short control blocks, which may be inlined (i.e. ``if(x) {return;}``), but enclosing braces must still be used

#. **Header Integrity**.
    * Keep header declarations as clean and sparse as possible, isolating active implementation details inside companion source files. Generic templates are an explicit exception to this constraint.
    * Each header file must be strictly standalone-includable and completely independent of external module-include sorting chains.
    * Every header must use ``#pragma once`` as its absolute first directive line.

#. **Cross-Platform Portability**.
    * Cross-platform execution compatibility is preferred but not universally mandated.
    * Linux-specific platform libraries or native syscalls may be utilized, provided they are clearly documented and isolated behind appropriate preprocessor platform guards.


