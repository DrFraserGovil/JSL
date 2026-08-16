.. _panopticon:

The Panopticon
==================

The Panopticon (the `All Watcher`) is a generalised manager class which unifies the other Watcher-objects under a single roof, allowing it to act as the central orchestrator of user-input for a larger system.
 
The Panopticon wraps the callback functions such that each Watcher does not execute the callback on their own thread (potentially causing horrendous data races), but instead report their data back to a central event queue, which the main thread of the Panopticon then works through, before going to sleep. This ensures that the Panopticon spends any time in which it is not reciving events or processing them asleep, removing pressure on the CPU.


.. jsl-class:: JSL::Async::Watcher::Panopticon
   :file: Async/Watcher/Panopticon.h


Instructions
---------------

.. doxygenstruct:: JSL::Async::Watcher::internal::Instruction
