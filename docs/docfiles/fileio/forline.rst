.. _forlinein:

Reading From File
=================

Basic File Reading
++++++++++++++++++++++++

The following functions allow a user to store the contents of a file directly into memory

.. jsl:: JSL::IO::getFile
	:file: FileIO/GetFile.h
	:command: doxygenfunction

.. doxygenfunction:: JSL::IO::getFileLines

.. doxygenfunction:: JSL::IO::getFileWords

File Processing
++++++++++++++++++

Often, a file is being read in not purely for storage, but because some form of operation is being performed on it. In such a case it is wasteful to store the entire string, and then postprocess it. The ``forLineIn`` routine and its descendents provide a way to operate on a file on a line-by-line basis. 

.. jsl:: JSL::IO::forLineIn
	:file: FileIO/forLineIn.H
	:command: doxygenfunction

.. doxygenfunction:: JSL::IO::forSplitLineIn


Direct Conversion
++++++++++++++++++++++

The file-read-in process is directly integrated with the :ref:`Parsing Library <parseto>`, allowing direct conversion of input data into a typed instance.

The first two overloads convert the entire line into the provided type, whilst the third overload allows arbitrary variadic types, and hence can convert the input file into a table.

.. doxygenfunction:: JSL::IO::forConvertedLineIn(const std::filesystem::path fileName, std::function<void(T)> callback)
.. doxygenfunction:: JSL::IO::forConvertedLineIn(const std::filesystem::path fileName, std::string_view delimiter, std::function<void(T)> callback)
.. doxygenfunction:: JSL::IO::forConvertedLineIn(const std::filesystem::path fileName, std::string_view delimiter, Func callback)