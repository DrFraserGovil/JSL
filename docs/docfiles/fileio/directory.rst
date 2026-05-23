.. _directory:

Directory Structure
========================

The ``std::filesystem`` library introduced in C++17 has made dealing with files and directories immeasurably easier, but it is couched in terms of iterators, which can sometimes be difficult and unintuitive to work with. The following structure is designed to provide a more object-oriented interface. 

Design Decisions
-------------------------

The following are some non-obvious design decisions it is useful to remember whilst using a ``JSL::IO::Directory``

Static Snapshot
+++++++++++++++++

Each ``IO::Directory`` instance represents a snapshot of the filesystem, taken at the moment of the objects initialisation. There is no live-updating, and so entities recorded within the Directory are not guaranteed to exist at a later point in time. 

The ``Rescan()`` method updates the snapshot (at the cost of invalidating all pointers and references to previous contents). For a 'live' file system, see the :ref:`async` library.

Globs & Regular Expressions
++++++++++++++++++++++++++++

When specifying patterns to exclude (during scanning) or include (during matching), there are two different interfaces, distinguished by the function signatures. 

Regular Expressions
/////////////////////

The 'native' method is for the user to construct a regular expression using the ``std::regex`` syntax, and to pass this in as an argument to a function accepting an ``std::regex`` signature. This is the most powerful and expressive means of filtering. 

Globs
/////////

Those familiar with unix-like file systems may find it significantly easier to construct filters based on a globbing-syntax; we provide support for this via the :ref:`glob` interface. This converts a glob-string into an equivalent regular expression. 

These functions are generally templates, accepting any string-like object (``std::string, std::string_view`` and ``const char *``).

The Directory Class
------------------------

.. jsl-class:: JSL::IO::Directory
	:file: FileIO/FileSystem.h