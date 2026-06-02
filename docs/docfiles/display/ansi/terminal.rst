.. _ansi-terminal:

ANSI Terminal Commands
========================

.. warning::
	The ANSI Terminal Commands *only* take effect if the output stream is capable of interpreting ANSI escape sequences. Non-ANSI-Capable streams (such as output files) will render them as incomprehensible gibberish. The ANSI status can be queries via the :cpp:class:`JSL::Display::Terminal::Environment`.

	Unlike :ref:`ansi-format`, these commands are **not** silenced when a bad Environment is detected, as the behaviour of (for example) moving a cursor vs. not is fundamentally different from not rendering a format command.  

CursorCommands
---------------------

We define a number of `CursorCommands` that can be inserted into a stdcout stream to garner the desired effect:

.. doxygentypedef:: JSL::Display::Terminal::CursorCommand


Usage
//////////

CursorCommands only take effect when piped to an ANSI-capable output stream.

.. code-block:: cpp

	using namespace JSL::Display::Terminal;
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

.. doxygenvariable:: JSL::Display::Terminal::Hide
.. doxygenvariable:: JSL::Display::Terminal::Show

Deletion Commands
##################

.. doxygenvariable:: JSL::Display::Terminal::Backspace
.. doxygenvariable:: JSL::Display::Terminal::ClearLine
.. doxygenvariable:: JSL::Display::Terminal::ClearScreen
.. doxygenvariable:: JSL::Display::Terminal::EraseAllLeft
.. doxygenvariable:: JSL::Display::Terminal::EraseAllRight

Movement Commands
#####################

.. doxygenvariable:: JSL::Display::Terminal::CursorUp
.. doxygenvariable:: JSL::Display::Terminal::ResetPosition

Dynamic Movement
--------------------

We also provide options to generate dynamic movement:

.. doxygenfunction:: JSL::Display::Terminal::MoveToColumn

.. doxygenenum:: JSL::Display::Terminal::Direction

.. doxygenfunction:: JSL::Display::Terminal::Move