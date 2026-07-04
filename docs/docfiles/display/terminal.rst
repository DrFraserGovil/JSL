.. _terminal:

Terminal Manipulation & Querying
==================================


TerminalCommands
---------------------

We define a number of `TerminalCommands` that can be inserted into a stdcout stream to garner the desired effect:

.. doxygentypedef:: JSL::Display::TerminalCommand


Usage
//////////

TerminalCommands only take effect when piped to an ANSI-capable output stream.


.. warning::
	The ANSI Terminal Commands *only* take effect if the output stream is capable of interpreting ANSI escape sequences. Non-ANSI-Capable streams (such as output files) will render them as incomprehensible gibberish. The ANSI status can be queries via the :cpp:class:`JSL::Display::Environment`.

	Unlike :ref:`ansi-format`, these commands are **not** silenced when a bad Environment is detected, as the behaviour of (for example) moving a cursor vs. not is fundamentally different from not rendering a format command.  

.. code-block:: cpp

	using namespace JSL::Display;
	std::string original = "Gello Worlf";
	std::cout << original << std::endl;
	std::cout << CursorUp << "H"; //overwrite the letter at this position
	std::cout << MoveToColumn(original.size()+1); /
	std::cout << Backspace; //delete final character
	std::cout << "d\n";

Output:

.. code-block:: shell-session

	Hello World


Visibility Commands
###################

.. doxygenvariable:: JSL::Display::HideCursor
.. doxygenvariable:: JSL::Display::ShowCursor


Deletion Commands
##################

.. doxygenvariable:: JSL::Display::Backspace
.. doxygenvariable:: JSL::Display::ClearLine
.. doxygenvariable:: JSL::Display::ClearScreen
.. doxygenvariable:: JSL::Display::EraseAllLeft
.. doxygenvariable:: JSL::Display::EraseAllRight

Movement Commands
#####################

.. doxygenvariable:: JSL::Display::CursorUp
.. doxygenvariable:: JSL::Display::ResetPosition

Dynamic Movement
////////////////////

We also provide options to generate dynamic movement:

.. doxygenfunction:: JSL::Display::MoveToColumn

.. doxygenenum:: JSL::Display::Direction

.. doxygenfunction:: JSL::Display::Move


Querying the Global Environment
---------------------------------

Most of the time it is not necessary to know the size of the terminal that the output is being streamed to; for some applications however, this information can allow for more precise graphical control.

The following class gives access to this information. It is designed only to be accessed through the :ref:`singleton access function <terminal_singleton>`, and has no public constructors.

.. warning::
	This code is currently linux-only. It will not work on WIN machines. 

.. jsl-class:: JSL::Display::GlobalEnvironment
	:file: Display/Terminal.h

Access Function
/////////////////////

.. _terminal_singleton:

.. jsl:: JSL::Display::Terminal()
	:file: Display/Terminal.h
	:command: doxygenfunction