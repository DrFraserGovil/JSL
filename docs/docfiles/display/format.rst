.. _ansi-format:

ANSI Formatting Commands
============================

Like the terminal commands, Formatting commands are -- at their heart -- strings of ANSI escape sequences. The structures below are wrappers around these strings to provide additional functionality and a more convenient interface.

.. warning::
	ANSI Escape sequences are only renderable in certain Environments (most modern terminal emulators, for example). If rendered into a file, however, the result is gibberish text. 

	The :cpp:class:`JSL::Display::Command` class replaces ANSI sequences with blank strings when a bad-environment is detecte, disabling formatting, but allowing for an easy user experience. This constrasts with the approach of :ref:`terminal`, where no such disabling occurs due to the fundamentally different behaviour.

Also note that ANSI control sequences have a non-zero length inside an ``std::string``, but have zero length when rendered to a terminal. This means that ``std::string::size`` is not a reliable indicator of string-length when ANSI control sequences are involved. We provide the :cpp:func:`JSL::String::trueSize` function for this purpose.

Sequence Functions
---------------------

.. dropdown:: Styles
	:color: primary
	:icon: italic

	.. jsl-meta::
		:file: Display/ANSI_Codes.h
		:namespace: JSL::Display::Display::Format

	.. doxygenfunction:: JSL::Display::Bold
	.. doxygenfunction:: JSL::Display::Faint
	.. doxygenfunction:: JSL::Display::Italics
	.. doxygenfunction:: JSL::Display::Underline
	.. doxygenfunction:: JSL::Display::Highlight
	.. doxygenfunction:: JSL::Display::Strike

.. dropdown:: Colours
	:color: primary
	:icon: paintbrush

	.. jsl-meta::
		:file: Display/ANSI_Codes.h
		:namespace: JSL::Display::Display::Format

	.. cpp:function:: Command JSL::Display::COLNAME(bool targetBackgroundCol = false)

	.. note::
		Substitute ``COLNAME`` with one of: :cpp:func:`Black`, :cpp:func:`Blue`,
		:cpp:func:`Cyan`, :cpp:func:`Green`, :cpp:func:`Purple`, :cpp:func:`Red`,
		:cpp:func:`Yellow`, :cpp:func:`White`.

	Set subsequent text to a 4-bit ANSI colour.

	:param targetBackgroundCol: If false (the default), the colour is used as the
		foreground colour. If true, it is assigned to the background.
	:returns: The associated ANSI command

	.. dropdown:: 4-bit Colours
		:color: secondary
		:icon: sparkle-fill

		.. doxygenfunction:: JSL::Display::Black
		.. doxygenfunction:: JSL::Display::Blue
		.. doxygenfunction:: JSL::Display::Cyan
		.. doxygenfunction:: JSL::Display::Green
		.. doxygenfunction:: JSL::Display::Purple
		.. doxygenfunction:: JSL::Display::Red
		.. doxygenfunction:: JSL::Display::Yellow
		.. doxygenfunction:: JSL::Display::White

	.. doxygenfunction:: JSL::Display::DefaultColour

	.. doxygenfunction:: JSL::Display::Colour

.. dropdown:: Resetting
	:color: primary
	:icon: iterations

	Each 'style' command can be disabled by passing the same command with the ``false`` argument (``Bold(false)`` disables boldface, for example). We also provide the following functions:

	.. jsl-meta::
		:file: Display/ANSI_Codes.h
		:namespace: JSL::Display::Format

	.. doxygenfunction:: JSL::Display::ResetAll
	.. doxygenfunction:: JSL::Display::Reset



.. _ansi-command:
    
Underlying Classes
---------------------

Each of the above functions returns a ``JSL::Display::Format`` object which contains the corresponding ANSI code, and an indicator as to which  :cpp:enum:`~JSL::Display::Element` it belongs to. The magic of these commands is that they can be :ref:`composited together <format-composite>` to create overall format schemas.

.. jsl-class:: JSL::Display::Format
	:file: Display/Format.h

ANSI Elements
++++++++++++++++++

Provided that they don't overwrite each other, ANSI sequences can be composited together: ``foreground->red`` and ``style->bold`` results in red-bold text. 

To ensure proper compositing, we split the commands into three elements:

.. jsl:: JSL::Display::Element
	:file: Display/Format.h
	:command: doxygenenum

ANSI Format Groups
++++++++++++++++++++


FormatGroup Class
-----------------------
 
.. _format-composite: 
 
.. jsl-class:: JSL::Display::FormatGroup
	:file: Display/Format.h

Operator Overloads
--------------------------
.. jsL:: JSL::Display::FormatType
	:file: Display/Format.h
	:command: doxygenconcept
	:namespace: JSL::Display::Format

.. doxygenfunction:: JSL::Display::operator+(const T & a, const U & b)
.. doxygenfunction:: JSL::Display::operator+(const T & a, std::string_view && b)
.. doxygenfunction:: JSL::Display::operator+(std::string_view && a, const T & b)