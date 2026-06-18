.. _display:

############
Display
############


The ``JSL::Display`` module contains classes and objects which enable the user to display information, and modify how that information is displayed.

.. toctree::
	:hidden:
	 
	display/format
	display/terminal
	display/progressbar

There are four main components of the Display submodule, all of which are nested within the ``JSL::Display`` namespace.

.. list-table::
	:header-rows: 1
	:widths: 20,80
	:class: no-wrap

	* - Submodule
	  - Contents

	* - :ref:`ANSI_Codes <ansi-format>`
	  - A thing that goes here
		 
	* - :ref:`ProgressBar.h <progress>`
	  - For displaying the progress of ongoing functions, with options for ANSI-based animated bars, and static bars suitable for file output
	
	* - :ref:`Terminal.h <terminal>` 
	  - For manipulating and querying the status of the output, if it is a 'modern terminal' environment.
	  
ANSI Escape Codes
==================

The majority of the submodules in ``JSL::Displaye`` use `ANSI Escape Codes`_ (AEC): a standardised way to interact with terminal output streams beyond simple text, enabling formatting, colouration, deletion of text and manual control of the cursor position. 

.. note::
	Cross platform support for AEC has historically been spotty. Linux and macOS systems should be fine; Microsoft has *finally* started supporting them (after 30 years!) starting in 2019. On Windows, ensure you are using `Windows Terminal`_ for the best experience.


.. _ANSI Escape Codes: https://en.wikipedia.org/wiki/ANSI_escape_code
.. _Windows Terminal: https://github.com/Microsoft/Terminal

Another consequence of this is that AEC functions only work within a terminal environment: they will result in onscreen-nonsense if piped into a filestream or other non-AEC-capable output. The :ref:`Display::Format <ansi-command>` function ensures that the formatting is simply omitted from style-commands; but we require a user to validate their output before using the :ref:`terminal` module, as no checks are performed here, as there is no 'safe equivalent' to simply omitting these commands. 