.. _ansi-format:

ANSI Formatting Commands
============================

Like the terminal commands, Formatting commands are -- at their heart -- strings of ANSI escape sequences. However, for JSL-aware stream managers (such as the :ref:`Logger <LOG>`), the structures below allow more fine grained control, such as resetting the font colour to default, whilst leaving the background colour untouched.

As well as using the predefined format commands below, we provide an interface 

.. toctree::
	elements
	command
	group
	:maxdepth: 1



Style
--------------------

.. jsl-meta::
	:file: Display/ANSI_Codes.h
	:namespace: JSL::Display::Format

.. doxygenfunction:: JSL::Format::Bold
.. doxygenfunction:: JSL::Format::Faint
.. doxygenfunction:: JSL::Format::Italics
.. doxygenfunction:: JSL::Format::Underline
.. doxygenfunction:: JSL::Format::Highlight
.. doxygenfunction:: JSL::Format::Strike

Colours
-----------------

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
	:icon: paintbrush

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
