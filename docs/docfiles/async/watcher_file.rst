.. _watcher-file:

File Watcher
===============

File Batches
---------------

Unlike the other two watchers, in which it makes sense for the captured data to be reported as a string, the results of the file watcher are better captured in a more structured fashion. The callback therefore acts on a *batch* of FileChange objects:

.. doxygenstruct:: JSL::Async::Watcher::FileChange

.. doxygenenum:: JSL::Async::Watcher::ChangeType
    
.. doxygenenum:: JSL::Async::Watcher::ObjectType


Watcher Class
----------------


.. jsl-class:: JSL::Async::Watcher::File
   :file: Async/Watcher/FileWatcher.h
    

