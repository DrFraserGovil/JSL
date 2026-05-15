.. _ansi-group:

ANSI Format Groups
====================


FormatGroup Class
-----------------------
.. jsl-class:: JSL::Format::FormatGroup
	:file: Display/Format.h

Operator Overloads
--------------------------
.. jsL:: JSL::Format::FormatType
	:file: Display/Format.h
	:command: doxygenconcept
	:namespace: JSL::Format

.. doxygenfunction:: JSL::Format::operator+(const T & a, const U & b)
.. doxygenfunction:: JSL::Format::operator+(const T & a, std::string_view && b)
.. doxygenfunction:: JSL::Format::operator+(std::string_view && a, const T & b)