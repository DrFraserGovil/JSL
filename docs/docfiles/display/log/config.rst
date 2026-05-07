.. _log-config:

Log Configuration
========================

The JSL::Log::Global Namespace
---------------------------------

The ``JSL::Log::Global`` namespace is designed to provide a safe corner for some global variables that are required to provide a global, configurable state with minimal interface overhead.

.. warning::
	As with all global variables, modifying objects in the this namespace comes with risks of non-local side-effects. Since this is a display-module, the risk is mostly that your output may get garbled.

	Generally, we encouage users to only modify the Config object at program start to avoid these side effects.

.. doxygennamespace:: JSL::Log::Global


The ConfigObject
----------------------

.. jsl-class:: JSL::Log::ConfigObjects

