.. _textwrap:

Text Wrapping
===========================

Text wrapping is the process where a long line of text is broken into multiple lines for display purposes. Generally this occurs automatically in the terminal, but there are instances where a user may wish to manually wrap text. 


.. toctree::
	folding_wrap
	folding_tabular
	:maxdepth: 1

Measurement
++++++++++++++

The above functions only work if one is able to accurately compute the *rendered* size of a line of text. When one enables the inclusion of :ref:`ANSI Escape Codes<display>` and tab characters, this is much more complex than ``string.size()``.

.. jsl:: JSL::String::trueSize(std::string_view)
	:file: Strings/Wrap.h
	:command: doxygenfunction

