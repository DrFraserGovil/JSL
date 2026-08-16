.. _socket_listener:

Socket Listener 
====================

The Socket Listener is used to recieve messages which have been sent via a Transmit or a Broadcaster object. However, the Listener::Read() call is blocking, and performs a hot-loop pinging of the socket, waiting for data to arrive. We therefore recommend that **users do not do the following**:

.. code-block:: cpp

   // A BAD EXAMPLE
   JSL::Async::Socket::Listener socket("test.socket")
   std::string msg;
   while (true)
   {
        r = socket.Read();
        if (r.Status == JSL::Async::Socket::Listener::ReadStatus::Success)
        {
            msg = r.Message;
            break;
        }
   }
   
   func(msg); // use the message

**This is highly discouraged**, as it is very inefficient. Instead, we recommend that the user use a :ref:`SocketWatcher <watchers>`, which automatically uses the system poller to sleep until a message is ready (and does so in a cross-platform fashion).

.. code-block:: cpp

    std::mutex sync;
    std::condition_variable block;

    std::string msg;
    JSL::Async::Watcher::Socket watcher("test.socket",false,[&](auto data){
        msg = std::move(data);
        block.notify_one();
    });
    watcher.Start();
    std::unique_lock lock(sync);
    block.wait(lock,[&](){!msg.empty()}); 
    watcher.Stop();
    func(msg);

.. jsl-class:: JSL::Async::Socket::Listener 
   :file: Async/Socket/Listener.h

This has the same behaviour as the first block of code, but the CPU spends most of its time asleep, rather than thrashing the OS with socket calls.

Helper Structs
-----------------

The MessageResult
/////////////////////

.. doxygenstruct:: JSL::Async::Socket::MessageResult
    
The ReadStatus
/////////////////////
    
.. doxygenenum:: JSL::Async::Socket::ReadStatus
    
 
