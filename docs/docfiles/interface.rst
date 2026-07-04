.. _interface:

Interface
==========

A common use-case for a code is to be able to modify its behaviour at launch-time by passing in configuration, either through command-line arguments, or configuration files. 

The ``Interface.h`` module provides a unified approach for configuring parameter values, with the aim of having minimal manual maintenance and duplication of information. For large scale parameter sets where, we recommend using our Domain-Specific Language, the `SSB`_, which allows a single 'class like' specification file to be parsed into a fully configurable object.  

.. _SSB: https://github.com/DrFraserGovil/SSB

.. toctree::
    :hidden:
     
    interface/aggregate 
    interface/context
    interface/commandline 
    interface/config 
    interface/parserbase 

Submodules
--------------

.. list-table::
    :header-rows: 1
    :widths: 20,80
    :class: no-wrap

    * - Submodule
      - Contents
    
    * - :ref:`Aggregate.h<aggregate>`
      - A CRTP wrapper which allows easy, compile time generation of a complex 'settings hub' with minimal user effort

    * - :ref:`Context.h<context>`
      - An object for providing context for accepted argument names and how their values should be tokenized

    * - :ref:`CommandLine.h<commandline>`
      - Group the values stored in ``argv`` into (positional) ``Commands`` and (flagged) ``Arguments``, either based on a simple parsing, or based on a provided context array.
            
    * - :ref:`Config.h<cmd-config>`
      - Read through a provided file, and store the results as if they were Arguments passed to the command line.
       

    * - :ref:`ParserBase.h<parserbase>`
      - The parent class of the `CommandLine` and `Config` objects.

Interface Philosophy
-----------------------

Both the CommandLine parser and the Config file parser operate on the same underlying mechanic: 'tokenize-then-parse'.

1. Both modules scan over their input (either an ``argv`` array, or a file)
2. The input is tokenized into pairs of strings; a ``key`` and a ``value``
    
   * With config files, this is simple; each key-value pair occupies their own line of the file, and they key is the first character sequence that occurs before the ``config-delim``; the value is any character sequence after this (including any subsequent ``config-delim``, but stripping out ``//``-delimited comments
   * The command line is more complex, and requires a :ref:`context map<context>` for advanced tokenisation. In simple terms each key is denoted by beginning with at least one ``-`` character, and the value is all subsequent characters until the next key. The Command Line also has ``commands`` as a third category, which are values not assigned to any key (the ``push`` in ``git push`` would be such an example).

3. These tokens are then stored as pure strings in a map
4. The user can then pull typed values out of the object using the ``Parse<T>`` member. This supports type-conversion from any type supported by the :ref:`JSL::String::ParseTo <parseto>` module.


Basic Usage
-----------------

.. code-block:: cpp

   // example.cpp
    #include <JSL.h>

    using namespace JSL::Interface;
    void Print(std::string name, ParserBase &parser)
    {
        std::cout << name << ":\n";
        std::cout << "\tVerbose: " << parser.Parse<bool>("verbose", false) << "\n";
        std::cout << "\tMessage: " << parser.Parse<std::string>("m", "No message") << "\n";
        std::cout << "\tDelay:   " << 2*parser.Parse<double>("delay", 0) << "\n"; 
        //  *2 just to prove it is a double!
    }

    int main(int argc, char **argv)
    {
        // declare the variables we are expecting
        ContextMap context({
            Context({"v", "verbose"}, KeyType::Flag),
            Context({"m", "message"}, KeyType::String),
            Context({"delay"}, KeyType::Value),

        });
        using namespace JSL::Interface;
        auto cmd = CommandLine(argc, argv, context);

        std::string configFile = "config.dat";
        auto config = Config(configFile, " ", context);

        auto joint = ConfigurableCommandLine(argc, argv, context);
        Print("CommandLine", cmd);
        Print("Config", config);
        Print("Joint", joint);
    }

If we also ensure the following file is on the path:

.. code-block:: text
    
    //config.dat
    message Hello world!
    delay 13.13

Then at runtime:

.. rst-class:: terminal-block
.. code-block:: text
    
    $ ./example -v -delay 2 -config config.dat
    CommandLine:
        Verbose: 1
        Message: No message
        Delay:   4
    Config:
        Verbose: 0
        Message: Hello world!
        Delay:   26.26
    Joint:
        Verbose: 1
        Message: Hello world!
        Delay:   4
         
    $ ./example -v -delay 2 -config config.dat -badkey
    terminate called after throwing an instance of 'std::runtime_error'
          what():  Bad Key  No parameter called 'badkey' exists

Note the following behaviours:
 
1. The context map provided a list of 'good' keys; so ``badkey`` was flagged as an error. ``config`` (and ``config-delim``, ``h`` and ``help``) are automatically registered, and so do not throw an error, even if they are unused.
2. The joint method ensures that the command line value of the delay (2) took priority over the config (13.13); but the config value was used where no command line was assigned (the ``message`` field).
