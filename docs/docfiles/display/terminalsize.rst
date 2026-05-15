.. _terminalsize:

Terminal Size
==================

Most of the time it is not necessary to know the size of the terminal that the output is being streamed to; for some applications however, this information can allow for more precise graphical control. 

.. warning::
	This code is currently linux-only. It will not work on WIN machines. 

.. jsl:: JSL::Terminal::GetDimensions
	:file: Display/Size.h
	:command: doxygenfunction

.. doxygenstruct:: JSL::Terminal::Size