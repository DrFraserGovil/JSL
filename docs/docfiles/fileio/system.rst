.. _filesystem:

Interacting with the Filesystem
=================================

The following functions make interacting with the filesystem easier: as with the :ref:`directory`, much of this functionality is available through the ``std::filesystem`` interface: this merely makes it more streamlined and easier to use. 

Making Directories
++++++++++++++++++++

.. jsl:: JSL::IO::mkdir
	:file: FileIO/Filesystem.h
	:command: doxygenfunction

Removing Objects
++++++++++++++++++

.. jsl:: JSL::IO::remove
	:file: FileIO/Filesystem.h
	:command: doxygenfunction

.. doxygenfunction:: JSL::IO::removeDirectory

Policies
++++++++++++++

.. doxygenenum:: JSL::IO::Policy

.. doxygenvariable:: JSL::IO::DefaultPolicy
    
Reporting System
+++++++++++++++++++

.. doxygenstruct:: JSL::IO::report
