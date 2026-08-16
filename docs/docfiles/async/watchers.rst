.. _watchers:

Async Watchers
================


.. toctree::
   :hidden:
    
   watcher_input
   watcher_file
   watcher_socket
    
.. list-table::
	:header-rows: 1
	:widths: 20,80
	:class: no-wrap

	* - Submodule
	  - Contents

	* - :ref:`Watcher::Input <watcher-input>`
	  - Applies a callback to all complete-lines which are entered into the standard input stream 
         
	* - :ref:`Watcher:::File <watcher-file>`
	  - Applies a callback to each batch of files which are changed in a watched directory
         
	* - :ref:`Watcher::Socket <watcher-socket>`
	  - Applies a callback to each line which is recieved on a Socket::Listener 

Motivation
----------------

A common use-case for asynchronous programming is to want to wait for an *event* to arrive, and then to apply some function to the data associated with that event (a 'callback' function).

The most naive way to do this might be something like this:

.. code-block:: cpp

   while (true)
   {
        std::set<Event> events;

        while (events.empty())
        {
            Check(events);
        }

        Process(events);

   }

That is, if you have some way of checking if an event is 'pending', then you enter into a *hot loop*, which checks if there is data, and if not, it keeps checking until there *is* data. 

This can be highly inefficient, however, as the CPU spends 100% uptime checking and rechecking if there is a new event. If the checking process requires dispatching system calls (i.e. checking the filesystem) then this can cause the OS to be 'thrashed', and lead to a significant performance degredation.

We might be tempted, then to make a simple modification to reduce this issue:


.. code-block:: cpp

   while (true)
   {
        std::set<Event> events;

        while (events.empty())
        {
            Check(events);
            std::this_thread::sleep_for(std::chrono::seconds(1)); //no CPU usage here
        }

        Process(events);

   }

A thread which is 'sleeping' takes next-to-no CPU cycles - and so this is a vast improvement, but now we have the problem that we have approximately a 1 second delay between an event occurring, and it being detected. In the case of a user interface, this might be intolerably long. We could of course reduce the sleep time -- but then we move back towards our problem of heavy CPU usage to achieve absolutely nothing. 

In short, we want some way to say ''sleep until there is data available''. This is what a Watcher is, and JSL provides three forms of Watcher:

