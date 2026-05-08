.. _ansi-format:

ANSI Formatting Commands
============================

.. jsl-meta::
	:file: Display/ANSI_Codes.h
	:namespace: JSL::Display::Format

Like the terminal commands, Formatting commands are -- at their heart -- strings of ANSI escape sequences. However, for JSL-aware stream managers (such as the :ref:`Logger <LOG>`), the structures below allow more fine grained control, such as resetting the font colour to default, whilst leaving the background colour untouched. 


Creating & Combining Formats
//////////////////////////////////

As well as using the predefined format commands below, we provide an interface 

.. toctree::
	elements
	command
	group
	:maxdepth: 1


Defined Commands
/////////////////////////////

.. warning:: 
	Certain combinations of fore/background colour and style are rejected by the terminal as not being readable, in which case it defaults to normal whilst those commands persist. 

	Which combinations of formats trigger this seems unpredictable and shell-dependent, but modifications to the background colour are the most commonly rejected. We recommend only changing the background colour if the Foreground colour is a predefined colour (i.e. not created via the RGB interface).



Style
--------------------

.. doxygenvariable:: JSL::Format::Bold
.. doxygenvariable:: JSL::Format::Faint
.. doxygenvariable:: JSL::Format::Italics
.. doxygenvariable:: JSL::Format::Underline
.. doxygenvariable:: JSL::Format::Highlight
.. doxygenvariable:: JSL::Format::Strike

Foreground Colour
--------------------

.. doxygenvariable:: JSL::Format::Black
.. doxygenvariable:: JSL::Format::Blue
.. doxygenvariable:: JSL::Format::Cyan
.. doxygenvariable:: JSL::Format::Green
.. doxygenvariable:: JSL::Format::Purple
.. doxygenvariable:: JSL::Format::Red
.. doxygenvariable:: JSL::Format::Yellow
.. doxygenvariable:: JSL::Format::White

Background Colour
--------------------

Removing Formats
////////////////////////

The standard method of clearing the format is to call the reset command, which returns the terminal to its default:

.. doxygenvariable:: JSL::Format::ResetAll

