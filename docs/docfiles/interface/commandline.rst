.. _commandline:

The Command Line Tokenizer
=============================


Usage
----------

There are a number of  ways that we support using this interface to assign string-input values from outside the program to typed-instances within the runtime. 


.. dropdown:: Option 1: Dumb Mode (not recommended)
    :color: warning
    :icon: alert
     
    The simplest instance is to avoid constructing a ContextMap, and rely on the automatic tokenisation.

    .. code-block:: cpp

        //example.cpp
        #include <JSL.h>
        int main(int argc, char **argv)
        {
            using namespace JSL::Interface;
            auto cmd = CommandLine(argc,argv);

            bool verbose = cmd.Parse("v", false); // or "verbose"
            auto file = cmd.Parse<std::filesystem::path>("f", "input.txt");
            std::string note = cmd.Parse<std::string>("note", "");

            if (verbose)
            {
                std::cout << "Executing commands " << JSL::String::makeFrom(cmd.Commands) << " on " << file << " with note '" << note << "'\n";
            }
        }
         
    This will work in simple cases:
    
    .. rst-class:: terminal-block
    .. code-block:: text 
     
        $ ./example -note test -f file.dat push -v
            Executing commands [push] on "file.dat" with note 'test'

    But without additional context, it will result in nonsense, and fail to throw errors when bad keys are passed, or the order results in ambiguous tokening:

    .. rst-class:: terminal-block
    .. code-block:: text 
     
        $ ./example -note Lorum ipsum dolor sit amet -v -f file.dat
            Executing commands [ipsum, dolor, sit, amet] on "file.dat" with note 'Lorum'
             
        $ ./example  -v push -f file.dat
            terminate called after throwing an instance of 'std::runtime_error'
              what(): Cannot convert string "push" to boolean
        
        $ ./example -not "That was a typo, so not*e* will be assigned" -v
            Executing commands [] on "input.txt" with note ''

    These are examples where it is important for the tokeniser to know what it is expecting, before tokenisation can begin. 

    Whilst 'dumb mode' may be best for small scale applications, or during initial testing phases, we do not suggest it for stable applications

.. dropdown:: Option 2: Up-Front Declaration
    :color: primary
    :icon: code-review 

    In this case, the user declares a ContextMap object, and then passes it to the CommandLine (or Config) object.
     
    .. code-block:: cpp
        
        //example.cpp
        #include <JSL.h>
        int main(int argc, char **argv)
        {
            using namespace JSL::Interface;
            ContextMap context({
                Context({"v", "verbose"}, KeyType::Flag), 
                Context({"f", "file"}, KeyType::Value),
                Context({"n", "note"}, KeyType::String)});
            auto cmd = CommandLine(argc, argv, context);
            bool verbose = cmd.Parse("v", false); // or "verbose"
            auto file = cmd.Parse<std::filesystem::path>("f", "input.txt");
            std::string note = cmd.Parse<std::string>("note", "");

            if (verbose)
            {
                std::cout << "Executing commands " << JSL::String::makeFrom(cmd.Commands) << " on " << file << " with note '" << note << "'\n";
            }
        } 

    This can then successfully parse the following complex examples, ensuring that:
     
    * The -note gets a long string
    * Aliases can be used interchangably with the 'canonical name'
    * The -v doesn't consume the 'push', which is correctly assigned into the commands
    * An error is thrown when an unregistered key is passesd

    .. rst-class:: terminal-block
    .. code-block:: text
        
        $ ./example -note Lorum ipsum dolor sit amet -verbose push -f file.dat
            Executing commands [push] on "file.dat" with note 'Lorum ipsum dolor sit amet'

        $ ./example -not "That was a typo" -v -f file.dat push
            terminate called after throwing an instance of 'std::runtime_error'
              what():  Bad Key  No parameter called 'not' exists

    The ContextMap also serves to act as a guard against alias collisions:

    
    .. code-block:: cpp
        
        //example.cpp
        #include <JSL.h>
        int main(int argc, char **argv)
        {
            using namespace JSL::Interface;
            ContextMap context({
                Context({"v", "verbose"}, KeyType::Flag), 
                Context({"vim", "v"}, KeyType::Value)});

                (...)
        } 
    
    At run time (since C++ can't yet handle compile-time string comparisons!)

    .. rst-class:: terminal-block
    .. code-block:: text
     
        ./example  -v push
        terminate called after throwing an instance of 'std::runtime_error'
          what():  Duplicate Alias  The key 'v' is already in use

.. dropdown:: Option 3: The Macro Wrapper
    :color: secondary
    :icon: cpu
    
    This functions almost identically to the above example, but without the need to up-front declare the names and types, as the ContextMap is generated at compile time by the macros.


    .. code-block:: cpp
        
        //example.cpp
        #include <JSL.h>
        #include <JSL/Interface/Macros.h> // this is *not* included otherwise
        int main(int argc, char **argv)
        {
            JSL_INTERFACE(argc, argv, auto commands,
                (bool, verbose, false, "v", "verbose"),
                (std::filesystem::path, file, "input.txt", "f", "file"),
                (std::string, note, "", "n", "note","and","arbitrary","number","of","aliases"));
            if (verbose)
            {
                std::cout << "Executing commands " << JSL::String::makeFrom(commands) << " on " << file << " with note '" << note << "'\n";
            }
        }  
    
    This compiles to identical code to "Option 2", but is easier for the developer to maintain as there is a 'single source of truth'.

    .. rst-class:: terminal-block
    .. code-block:: text
        
        $ ./example -note Lorum ipsum dolor sit amet -v -f file.dat push
            Executing commands [push] on "file.dat" with note 'Lorum ipsum dolor sit amet'

        $ ./example -not "That was a typo" -v -f file.dat push
            terminate called after throwing an instance of 'std::runtime_error'
              what():  Bad Key  No parameter called 'not' exists

    .. warning:: 
        This method has some fairly major drawbacks.
         
        1. No more than **eight** (8) arguments can be declared within the macro
        2. It's a macro wrapper around templated functions: there may be some extremely incomprehensible error messages if you make a mistake
        3. The macro pollutes the global namespace with a number of preprocessor directives (all preceeded by ``JSL_``), and loose variables (preceeded by ``__JSL_``). As a result, we do not import this automatically with the rest of the JSL; it must be manually #included.


.. dropdown:: Option 4: Aggregators
    :color: primary
    :icon: duplicate

    These first three approaches are suitable when the goal is to configure a handful of 'loose variables' in the main function. They (especially the Macro Wrapper) are not suitable for (for example) simulational codes, where the goal is to configure an aggregate 'settings' object which may contain many dozens - even hundreds- of variables that may configured.

    In such a case, it is of vital importance to minimise the duplication of boilerplate code, and the possibility of semantic drift between where a variable is declared and stored, and where the associated metadata can be found. 

Notable Features
++++++++++++++++++++++++++++++++++++

.. dropdown:: Dash-Stripping & Flag Clustering
    :color: secondary
    :icon: paperclip

    In Unix-like applications it is common for a single-dash key to be a flag (requiring no input) and a double-dash to be a value (requiring input). In contrast, we dash-normalise all inputs, such that the number of dashes which are prefixed to a key is irrelevant, so long as there is at least one. ``-v``, ``--v`` and ``---------v`` are all equivalent.

    We do, however, permit 'flag clustering', in which multiple, single-character flags can be clustered together into a single call; so that ``-abc`` is equivalent to ``-a -b -c``.

    In order for a clustering to be valid:

    1. The full-name must not be a valid alias in its own right (i.e. if ``abc`` had been assigned to its own variable)
    2. All members of the cluster must be valid, single-character aliases with ``KeyType::Flag``.
    3. Each flag occurs no more than once in the cluster

 
    .. code-block:: cpp
        
        //example.cpp
        #include <JSL.h>
        #include <JSL/Interface/Macros.h> // this is *not* included otherwise
        int main(int argc, char **argv)
        {
            JSL_INTERFACE(argc, argv, auto commands,
                (bool, flag1, false, "a"),
                (double, val, 0, "ca"),
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
         
        $ ./example -ca //ca is a double alias, so this will fail
            error: value 'ca' requires a value
             
CommandLine Class
---------------------
 
.. jsl-class:: JSL::Interface::CommandLine
   :namespace: JSL::Interface
   :file: Interface/CommandLine.h
    

