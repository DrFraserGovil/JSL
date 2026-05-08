.. _log-config:

Log Configuration
========================

Global Configuration
----------------------

To ensure that there is a 'single source of truth' for the formatting of the Log :cpp:class:`~JSL::Log::Core` , the associated properties are stored within the :cpp:class:`JSL::Log::Config` class, which is wrapped in a Meyers Singleton and has a private constructor. 

The only way to interface with the object is through the :cpp:func:`~JSL::Log::Config::Global` static method, ensuring that only one Config object can exist at a time. Modifying the variables of this object modifies the behaviour of all subsequent calls to :ref:`LOG <LOG>`.

.. warning::
	As with all global variables, modifying members of the `Global` instance comes with risks of non-local side-effects. Since this is a display-module, the risk is mostly that your output may get garbled.

	Generally, we encourage users to only modify the Config object at program start to avoid these side effects.


The Log::Config
----------------------

.. jsl-class:: JSL::Log::Config

