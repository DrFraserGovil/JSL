.. _context:

Parser Context
=================

When parsing the command line, the naive method is to assume that keys are distinguished solely by beginning with a dash, and any non-dash character which follows is therefore a value associated with the previous key. This is made more complex when one allows, for example, key-only flags (such as ``-q`` which might be an alias for ``--quiet true``) .

Equally, if one wants the ability to throw an error if an unknown command is processed, it is required for the code to have - up front - a known registry of parameters, and the parse-type that will be applied to them.

.. toctree::
    :maxdepth: 1
     
    contextlone     
    contextmap 
    parsetypes

