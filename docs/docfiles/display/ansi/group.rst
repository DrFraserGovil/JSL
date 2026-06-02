.. _ansi-group:

ANSI Format Groups
====================


FormatGroup Class
-----------------------
.. jsl-class:: JSL::Display::Format::FormatGroup
	:file: Display/Format.h

Operator Overloads
--------------------------
.. jsL:: JSL::Display::Format::FormatType
	:file: Display/Format.h
	:command: doxygenconcept
	:namespace: JSL::Display::Format

.. doxygenfunction:: JSL::Display::Format::operator+(const T & a, const U & b)
.. doxygenfunction:: JSL::Display::Format::operator+(const T & a, std::string_view && b)
.. doxygenfunction:: JSL::Display::Format::operator+(std::string_view && a, const T & b)