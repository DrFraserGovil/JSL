.. _log-levels:

==============
Log Levels
==============

Each ``LOG`` macro creates a ``Log::Core`` object assigned one of the following levels, which determines if it will be output to the terminal or suppressed.


.. warning::
	
	These names are entered into the global namespace (to make Logging as streamlined as possible). If naming conflicts arise, it will be necessary to disambiguate manually:
	
	``LOG(LogLevel::WARN) << msg``

.. doxygenenum:: LogLevel
   :project: JSL


