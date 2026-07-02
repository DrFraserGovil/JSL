.. _commandline:

The Command Line Tokenizer
=============================

Notable Features
----------------------

Dash-Stripping & Flag Clustering
++++++++++++++++++++++++++++++++++++

In Unix-like applications it is common for a single-dash key to be a flag (requiring no input) and a double-dash to be a value (requiring input). In contrast, we dash-normalise all inputs, such that the number of dashes which are prefixed to a key is irrelevant, so long as there is at least one. ``-v``, ``--v`` and ``---------v`` are all equivalent.

We do, however, permit 'flag clustering', in which multiple, single-character flags can be clustered together into a single call; so that ``-abc`` is equivalent to ``-a -b -c``.

In order for a clustering to be valid:

1. The full-name must not be a valid alias in its own right (i.e. if ``abc`` had been assigned to its own variable)
2. All members of the cluster must be valid, single-character aliases with ``KeyType::Flag``.
3. Each flag occurs no more than once in the cluster

.. dropdown:: Example
 
    .. code-block:: cpp
        
        //example.cpp
        #include <JSL.h>
        #include <JSL/Interface/Macros.h> // this is *not* included otherwise
        int main(int argc, char **argv)
        {
            JSL_INTERFACE(argc, argv, auto commands,
                (bool, flag1, false, "a"),
                (bool, flag2, false, "b"),
                (bool, flag3, false, "c"));
            std::cout << "a: " << flag1 << " b: " << flag2 << " c: " << flag3 << "\n";
        } 
         
    .. rst-class:: terminal-block
    .. code-block:: text
     
        $ ./example
        a: 0 b: 0 c: 0
         
        $ ./example -a
        a: 1 b: 0 c: 0
         
        $ ./example -a -b
        a: 1 b: 1 c: 0
         
        $ ./example -ab
        a: 1 b: 1 c: 0
         
        $ ./example -cab
        a: 1 b: 1 c: 1

CommandLine Class
---------------------
 
.. jsl-class:: JSL::Interface::CommandLine
   :namespace: JSL::Interface
   :file: Interface/CommandLine.h
    

