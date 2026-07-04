.. _help:

Aggregator Help
=======================


All ContextMaps automatically reserve the ``-h`` and ``-help`` boolean "help mode" indicators. Whenever an Aggregator detects that such a flag (or a ``help`` command) has been sent, it constructs and prints a ``HelpGroup``, which is flushed to the display, before ``exit(0)`` is called.

Metadata
-----------

This metadata is stored in the Hep 

.. jsl-class:: JSL::Interface::HelpMetaData
    :file: Interface/Help.h
     
  
Help Message Formatter
-------------------------


.. jsl-class:: JSL::Interface::internal::HelpGroup
   :file: Interface/Help.h

