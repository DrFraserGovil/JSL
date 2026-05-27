.. _ansi-terminal:

ANSI Terminal Commands
========================

.. warning::
	The ANSI Terminal Commands *only* take effect if the output stream is capable of interpreting ANSI escape sequences. Non-ANSI-Capable streams (such as output files) will render them as incomprehensible gibberish. The ANSI status can be queries via the :cpp:class:`JSL::Terminal::Environment`.

	Unlike :ref:`ansi-format`, these commands are **not** silenced when a bad Environment is detected, as the behaviour of (for example) moving a cursor vs. not is fundamentally different from not rendering a format command.  

CursorCommands
---------------------

We define a number of `CursorCommands` that can be inserted into a stdcout stream to garner the desired effect:

.. doxygentypedef:: JSL::Terminal::CursorCommand


Usage
//////////

CursorCommands only take effect when piped to an ANSI-capable output stream.

.. code-block:: cpp

	using namespace JSL::Terminal;
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

.. doxygenvariable:: JSL::Terminal::Hide
.. doxygenvariable:: JSL::Terminal::Show

Deletion Commands
##################

.. doxygenvariable:: JSL::Terminal::Backspace
.. doxygenvariable:: JSL::Terminal::ClearLine
.. doxygenvariable:: JSL::Terminal::ClearScreen
.. doxygenvariable:: JSL::Terminal::EraseAllLeft
.. doxygenvariable:: JSL::Terminal::EraseAllRight

Movement Commands
#####################

.. doxygenvariable:: JSL::Terminal::CursorUp
.. doxygenvariable:: JSL::Terminal::ResetPosition

Dynamic Movement
--------------------

We also provide options to generate dynamic movement:

.. doxygenfunction:: JSL::Terminal::MoveToColumn

.. doxygenenum:: JSL::Terminal::Direction

.. doxygenfunction:: JSL::Terminal::Move