.. _interface:

Command-Line Interface & Configuration
========================================

A common use-case for a code is to be able to modify its behaviour at launch-time by passing in configuration, either through command-line arguments, or configuration files. 

The ``Interface.h`` module provides a unified approach for configuring parameter values, with the aim of having minimal manual maintenance and duplication of information. For large scale parameter sets where, we recommend using our Domain-Specific Language, the `SSB`_, which allows a single 'class like' specification file to be parsed into a fully configurable object.  

.. _SSB: https://github.com/DrFraserGovil/SSB

.. toctree::
	:hidden:

	interface/commandline 
	interface/config 
	
.. list-table::
    :header-rows: 1
    :widths: 20,80
    :class: no-wrap

    * - Submodule
      - Contents

    * - :ref:`CommandLine.h<commandline>`
      - Group the values stored in ``argv`` into (positional) ``Commands`` and (flagged) ``Arguments``, either based on a simple parsing, or based on a provided context array.
   	        
    * - :ref:`Config.h<cmd-config>`
      - Read through a provided file, and store the results as if they were Arguments passed to the command line.
         
         
       
