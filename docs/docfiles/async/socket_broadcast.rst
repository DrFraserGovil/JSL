.. _socket_broadcast:

Socket Broadcaster 
====================

The Broadcaster establishes a more consistent connection to a socket (though it does not maintain an open pipe). It is therefore more efficient for multiple rounds of communication to the same socket, as the existence checks etc. do not need to be repeated. 

.. jsl-class:: JSL::Async::Socket::Broadcaster 
   :file: Async/Socket/Broadcaster.h
