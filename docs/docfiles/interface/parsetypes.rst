.. _parsetypes:


Parse Types
=================



The following keytypes are defined, each indicating slightly differing behaviour in how tokens are consumed from the commandline 

.. jsl:: JSL::Interface::KeyType
   :command: doxygenenum
   :file: Interface/KeyTypes.h 


We also provide a number of template-cast functions, which return compile-time evaluated mappings between ``typenames`` and ``KeyTypes``; with the default being ``KeyType::Value``.

.. doxygenstruct:: JSL::Interface::MapTypeToKey
