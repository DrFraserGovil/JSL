.. _log-core:

Log Core
===========


.. jsl-class:: JSL::Log::Core
	:file: JSL/Display/Log/Core.h
	:namespace: JSL::Log

	The Log::Core is a transient object created by the LOG macro. It performs internal formatting on the stream passed to it, and when the object is destroyed, it flushes the output to STDCOUT.

