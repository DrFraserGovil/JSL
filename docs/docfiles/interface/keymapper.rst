.. _keymap:

Key Aliases & Context
=======================

When parsing the command line, the naive method is to assume that keys are distinguished solely by beginning with a dash, and any non-dash character which follows is therefore a value associated with the previous key. This is made more complex when one allows, for example, key-only flags (such as ``-q`` which might be an alias for ``--quiet true``) 

.. jsl-class:: JSL::Interface::KeyMapper
   :file: Interface/KeyMapper.h
    
The KeyTypes
--------------

The following keytypes are defined, each indicating slightly differing behaviour in how tokens are consumed from the commandline 

.. jsl:: JSL::Interface::KeyType
   :command: doxygenenum
   :file: Interface/KeyMapper.h 




