.. _log-indent: 
    
Log Indenting
=================
   
It is common to want to structure the logging output by 'nesting' logs within a heirarchy, with varying number of indents. This can be achieved by either hardcoding a number of indent characters, or transporting around an 'indent' parameter, and calling:

.. code-block:: cpp

   void subroutine(int cmds, size_t indent)
   {
       LOG(INFO) << std::string(indent,'\t') << "Submessage " << cmds;

   }

   void routine()
   {
       LOG(INFO) << "Top level message:";
       size_t indent = 1;
       for (int i = 0; i < 2; ++i)
       {
          LOG(INFO) << std::string(indent,'\t') << "Message " << i;
          subroutine(3*i*i,indent+1);
          subroutine(0,indent+1);
       }
       LOG(INFO) << "End message";
    }

Giving:
 
.. code-block:: shell-session
 
   [INFO] Top level message
   [INFO]     Message 1
   [INFO]         Submessage 3
   [INFO]         Submessage 0
   [INFO]     Message 2
   [INFO]         Submessage 12
   [INFO]         Submessage 0
   [INFO] End message

However this will get clumsy if over-used, and requires transporing an argument around purely for display purposes. Instead, the JSL offers a way to add in a global indent which persists on all messages, until the Indent object goes out of scope:


.. code-block:: cpp
    
   void subroutine(int cmds, size_t indent)
   {
       LOG(INFO) << "Submessage " << cmds;

   }

   void routine()
   {
       LOG(INFO) << "Top level message:";
       { //scoping for the indent
           auto indent = JSL::Log::Indent();       // +1 indent
           for (int i = 0; i < 5; ++i)
           {
              LOG(INFO)  << "Message " << i; 
              auto subindent = JSL::Log::Indent(); // +1 indent
              subroutine(3*i*i,indent+1);
              subroutine(0,indent+1);
           }                                       // -1 indent (subindent goes out of scope)
       }                                           // -1 indent (indent goes out of scope)
       LOG(INFO) << "End message";

    }

This has identical output.


.. jsl-class:: JSL::Log::Indent
   :file: Log/Indent.h

