ANSI Elements
==================

Provided that they don't overwrite each other, ANSI sequences can be composited together: ``foreground->red`` and ``style->bold`` results in red-bold text. 

To ensure proper compositing, we split the commands into three elements:

.. jsl:: JSL::Display::Element
	:file: Display/Format.h
	:command: doxygenenum



Usage 
////////

These enums are largely hidden from the user (they are used internally to encode, i.e. the difference between :cpp:func:`JSL::Display::Black` and :cpp:func:`JSL::Format::Bold`), but they may be encountered in the :cpp:func:`JSL::Format::Reset` function.