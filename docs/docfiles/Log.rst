.. _log:

----------
Logging
----------

The Logging submodule allows the developer to output text to the output stream with preset formatting and allow the user to filter out certain messages at runtime. 


Inside the Logger
===================

.. toctree::
	log/levels
	log/config
	log/core
	:maxdepth: 1



The LOG Macro
==============

The central object of the Logging module is the ``LOG`` macro, which acts as a wrapper around the internal  :ref:`log-core` object.


.. doxygendefine:: LOG
	

.. note:: 

	The LOG macro has the potential to pollute the global namespace. In order to prevent this, the library allows the user to avoid including the logger (though it may be used by the internal translation units):
	
	.. code-block:: cpp

		#define JSL_REMOVE_LOG
		#include <JSL.h> 




Usage
=============

.. code-block:: cpp

	int nWarn = 21;
	JSL::Log::Global.Config.SetLevel(INFO); //set the global output level

	LOG(WARN)  << "This is a warning message! You have done " << nWarn << " bad things";
	LOG(DEBUG) << "This is a very detailed debug message that's not useful at runtime";
	LOG(INFO)  << "This is some useful information";

Output:

.. code-block:: shell-session

	[WARN] This is a warning message! You have done 21 bad things
	[INFO] This is some useful information

If the terminal is ANSI-capable, the Config will also format these messages in different colours.