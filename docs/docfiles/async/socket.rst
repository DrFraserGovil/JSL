.. _socket:

Sockets
===========

The JSL supports Inter-Process Communications (IPC) through the ``JSL::Async::Socket`` submodule, which provides an interface to the Unix Domain Sockets (which are supported by Windows :math:`\geq10`).


.. toctree::
   :hidden:

   socket_base
   socket_broadcast
   socket_listener
   socket_transmit


.. list-table::
   :header-rows: 1
   :widths: 40,60
   :class: no-wrap

   * - Submodule
     - Contents

   * - :ref:`Socket Broadcasting <socket_broadcast>`
     - A class for sending multiple messages to the same socket
        
   * - :ref:`Socket Listening <socket_listener>`
     - A class for reading data from a socket 
         
   * - :ref:`Socket Trasmitting <socket_transmit>`
     - A function for one-shot messaging to a socket
        
   * - :ref:`Socket Internals <socket_base>`
     - The internal base class which ensures Broadcaster and Listener perform the same validation 

