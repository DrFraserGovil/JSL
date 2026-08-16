.. _async:

Asynchronous & Parallel Execution
=====================================

.. toctree::
   :hidden:

   async/pool
   async/socket
   async/watchers

The following modules provide various means of Asynchronous task execution:

.. list-table::
	:header-rows: 1
	:widths: 20,80
	:class: no-wrap

	* - Submodule
	  - Contents

	* - :ref:`Pool.h <parpool>`
	  - Direct Asynchronous-Task dispatch, and efficient distribution of tight for-loops
         
	* - :ref:`Socket.h <socket>`
	  - Transmit and Recieve Inter-Process Communications via Unix Domain Sockets 
         
	* - :ref:`Watcher.h <watchers>`
	  - A submodule containing Watchers which apply callbacks to system-level changes 

.. admonition:: Cross-Platform Support 

    The Async module is unique to the JSL in that it is tightly coupled to the platform on which it is running. We have validated that module functions on Linux, Windows and BSD (including macOS) platforms.

What do we mean by 'Parallel'?
################################

There are multiple different techniques which lie under the umbrella of Asynchronous/Parallel computing, so it is worthwhile being explicit about what it is we mean. 

The simplest model of parallel computing is the *shared memory* style - often called OpenMP style or, in our terminology, a *worker pool*. Under this model of computing, the workers are all dispatched from the same process, and have total memory access: any worker may modify the memory any other worker is touching (often to disastrous effects).

The advantage of this is simplicity: the code may simply say "worker 1 will handle this task, worker 2 this task, whilst I do something else" -- and then wait for all three tasks to be complete. However, this model cannot scale arbitrarily: the maximum number of concurrent workers is the number of cores on a single machine. Care must also be taken to avoid *race-conditions* (multiple workers modifying/reading from the memory at the same instant) and *deadlocks* (when the process to prevent race conditions malfuncitons, and no workers can proceed).

This contrasts with the *distributed worker* model (also known as *MPI style*), wherin each worker is a separate running process. These workers have their own memory which others cannot touch, except through inter-worker communication, via some messaging interface. This is much more cumbersome to write, but allows for arbitrary scaling.

.. warning::

   When we talk about parallel computing from now on, we shall mean this first definition: multiple tasks being executed simultaneously by the same process.

   The second form of parallel computing is achieved using Inter-Process Communication, which can be handled through the Socket library we provide; but we generally won't refer to this usage explicitly.

Existing Threading
///////////////////////

Since C++11, C++ has supported the ``<thread>`` library, which forms the basis of this module. The main limitation of using ``std::thread`` naively is that 'spinning up' a thread incurs a significant overhead; the aim of this module is therefore to hold a number of pre-generated threads, and allow them to perform multiple tasks, with minimal overhead.


