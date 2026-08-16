.. _watcher-socket:

Socket Watcher
===============

As mentioned in :ref:`the Socket::Listener documentation <socket_listener>`, pinging the socket in a hot loop is extremely taxing on the OS. The SocketWatcher (or the Panopticon) should be preferred in almost every instance where the user is not building their own polling system. 

.. jsl-class:: JSL::Async::Watcher::Socket
   :file: Async/Watcher/SocketWatcher.h
    

