.. _fileio:

.. cpp::module:: IO

File I/O
=================

The ``JSL::IO`` module provides classes and functions for common filesystem interactions. Where the standard library (particularly ``std::filesystem``) prioritises generality and power, ``JSL::IO`` prioritises brevity and ease of use — at the cost of some flexibility.

.. toctree::
	:hidden:
	 
	fileio/directory
	fileio/write
	fileio/system
	fileio/forline
	fileio/getfile
	fileio/glob
	fileio/pipe
	fileio/vault


.. list-table::
	:header-rows: 1
	:widths: 20,80
	:class: no-wrap

	* - Submodule
	  - Contents

	* - :ref:`Directory.h <directory>` 
	  - Provides an object-oriented view of a targeted filesystem, in contrast to the iterator approach of the standard library

	* - :ref:`FileWriters.h <file-write>`
	  - Provides streamlined filewriting interfaces & template interfaces for easily writing vectors and matrices to file.

	* - :ref:`FileSystem.h <filesystem>`
	  - Provides cross-platform wrappers and improved error handling around 

	* - :ref:`ForLineIn.h<forlinein>`
	  - Provides a functional-program style interface for line-by-line callbacks on files without needing to read the file into memory. 

	* - :ref:`GetFile.h<getfile>`
	  - Provides a number of simple wrappers for reading a file into memory in a variety of formats.

	* - :ref:`Glob.h<glob>`
	  - A cross platform implementation of ``fnmatch``/``glob`` syntax

	* - :ref:`PipedInput.h<pipe>`
	  - Provides a means of detecting and processing input which is being piped into the ``std::cin`` of the program.

	* - :ref:`Vault.h <vault>`
	  - Provides a read/write interface to file archives, allowing multiple 'files' to be stored as a single block.