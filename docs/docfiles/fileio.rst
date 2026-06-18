.. _fileio:

.. cpp::module:: IO

File I/O
=================

The ``JSL::IO`` module provides classes and functions for common filesystem interactions. Where the standard library (particularly ``std::filesystem``) prioritises generality and power, ``JSL::IO`` prioritises brevity and ease of use — at the cost of some flexibility.

.. toctree::
	:hidden:
	 
	fileio/directory
	fileio/write
	fileio/forline
	fileio/pipe
	fileio/glob
	fileio/system
	fileio/archiver


.. list-table::
	:header-rows: 1
	:widths: 20,80
	:class: no-wrap

	* - Submodule
	  - Contents

	* - :ref:`Directory.h <directory>` 
	  - Provides an object-oriented view of a targeted filesystem, in contrast to the iterator approach of the standard library