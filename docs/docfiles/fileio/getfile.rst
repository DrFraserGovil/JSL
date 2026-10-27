.. _getfile:

Basic File Reading
===================



The following functions allow a user to store the contents of a file directly into memory

.. jsl:: JSL::IO::getFile
	:file: FileIO/GetFile.h
	:command: doxygenfunction

.. doxygenfunction:: JSL::IO::getFileLines(const std::filesystem::path fileName)
    
.. doxygenfunction:: JSL::IO::getFileLines(const std::filesystem::path fileName, size_t nLines, bool includeBlank = true)

.. doxygenfunction:: JSL::IO::getFileWords
