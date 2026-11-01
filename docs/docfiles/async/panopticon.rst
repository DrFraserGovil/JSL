.. _panopticon:

The Panopticon
==================

The Panopticon (the `All Watcher`) is a generalised manager class which unifies the other Watcher-objects under a single roof, allowing it to act as the central orchestrator of user-input for a larger system.
 
The Panopticon wraps the callback functions such that each Watcher does not execute the callback on their own thread (potentially causing horrendous data races), but instead report their data back to a central event queue, which the main thread of the Panopticon then works through, before going to sleep. This ensures that the Panopticon spends any time in which it is not reciving events or processing them asleep, removing pressure on the CPU.


Pausing the input
-------------------

A callback sometimes needs to read the standard input itself, for example to ask a follow-up question or run an interactive :code:`Console`. While the Panopticon's input watcher is running, it would race the callback for every line, so the callback first pauses it, and resumes it afterwards:

.. code-block:: cpp

   panopticon.SetInputCallback([&](std::string line) {
       if (line == "ask")
       {
           panopticon.PauseInput(); // nothing else reads the standard input until ResumeInput()
           std::string answer;
           std::getline(std::cin, answer);
           panopticon.ResumeInput();
       }
   });

The other watchers keep running while the input is paused; their events wait in the queue until the callback returns.

.. jsl-class:: JSL::Async::Watcher::Panopticon
   :file: Async/Watcher/Panopticon.h


Instructions
---------------

.. doxygenstruct:: JSL::Async::Watcher::internal::Instruction
