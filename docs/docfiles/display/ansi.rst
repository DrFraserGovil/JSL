.. _ansi-display:

ANSI Codes
===============

`ANSI Escape Codes`_ (AEC) are a standardised way to interact with terminal output streams beyond simple text, enabling formatting, colouration, deletion of text and manual control of the cursor position. 

.. note::
	Cross platform support for AEC has historically been spotty. Linux and macOS systems should be fine; Microsoft has *finally* started supporting them (after 30 years!) starting in 2019. On Windows, ensure you are using `Windows Terminal`_ for the best experience.


.. _ANSI Escape Codes: https://en.wikipedia.org/wiki/ANSI_escape_code
.. _Windows Terminal: https://github.com/Microsoft/Terminal

We subdivide the commands into two groups:

.. toctree::
	ansi/format
	ansi/terminal
	:maxdepth: 2
