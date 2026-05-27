.. _ansi-format:

ANSI Formatting Commands
============================

Like the terminal commands, Formatting commands are -- at their heart -- strings of ANSI escape sequences. The structures below are wrappers around these strings to provide additional functionality and a more convenient interface.

.. warning::
	ANSI Escape sequences are only renderable in certain Environments (most modern terminal emulators, for example). If rendered into a file, however, the result is gibberish text. 

	The :cpp:class:`JSL::Format::Command` class replaces ANSI sequences with blank strings when a bad-environment is detecte, disabling formatting, but allowing for an easy user experience. This constrasts with the approach of :ref:`ansi-terminal`, where no such disabling occurs due to the fundamentally different behaviour.

Also note that ANSI control sequences have a non-zero length inside an ``std::string``, but have zero length when rendered to a terminal. This means that ``std::string::size`` is not a reliable indicator of string-length when ANSI control sequences are involved. We provide the :cpp:func:`JSL::String::trueSize` function for this purpose.

Sequence Functions
---------------------

.. dropdown:: Styles
	:color: primary
	:icon: italic

	.. jsl-meta::
		:file: Display/ANSI_Codes.h
		:namespace: JSL::Display::Format

	.. doxygenfunction:: JSL::Format::Bold
	.. doxygenfunction:: JSL::Format::Faint
	.. doxygenfunction:: JSL::Format::Italics
	.. doxygenfunction:: JSL::Format::Underline
	.. doxygenfunction:: JSL::Format::Highlight
	.. doxygenfunction:: JSL::Format::Strike

.. dropdown:: Colours
	:color: primary
	:icon: paintbrush

	.. jsl-meta::
		:file: Display/ANSI_Codes.h
		:namespace: JSL::Display::Format

	.. cpp:function:: Command JSL::Format::COLNAME(bool targetBackgroundCol = false)

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

		.. doxygenfunction:: JSL::Format::Black
		.. doxygenfunction:: JSL::Format::Blue
		.. doxygenfunction:: JSL::Format::Cyan
		.. doxygenfunction:: JSL::Format::Green
		.. doxygenfunction:: JSL::Format::Purple
		.. doxygenfunction:: JSL::Format::Red
		.. doxygenfunction:: JSL::Format::Yellow
		.. doxygenfunction:: JSL::Format::White

	.. doxygenfunction:: JSL::Format::DefaultColour

	.. doxygenfunction:: JSL::Format::Colour

.. dropdown:: Resetting
	:color: primary
	:icon: iterations

	Each 'style' command can be disabled by passing the same command with the ``false`` argument (``Bold(false)`` disables boldface, for example). We also provide the following functions:

	.. jsl-meta::
		:file: Display/ANSI_Codes.h
		:namespace: JSL::Format

	.. doxygenfunction:: JSL::Format::ResetAll
	.. doxygenfunction:: JSL::Format::Reset


Internal Documentation
---------------------------
.. toctree::
	elements
	command
	group
	:maxdepth: 1

