.. _aggregate:


Settings Aggregator
=======================

The :ref:`CommandLine<commandline>` and :ref:`Config<cmd-config>` interfaces are suitable when the goal is to configure a handful of 'loose variables' in the main function. However, they are largely unsuitable for (for example) simulational codes, where the goal is to configure an aggregate 'settings' object which may contain many dozens - even hundreds- of variables that may configured.

In such a case, it is of vital importance to minimise the duplication of boilerplate code, and the possibility of semantic drift between where a variable is declared and stored, and where the associated metadata can be found. 

This is the goal of the Aggregator construct, a `Curiously Recurring Template Pattern`_ aimed to act as a wrapper class for a user-defined data-holding class. 
 
 
.. _Curiously Recurring Template Pattern: https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
 
.. dropdown:: Why Use An Aggregator
    :color: success
    :icon: question

    Consider the following constructs:

    .. code-block:: cpp
     
        // example.cpp
        #include <JSL.h>

        using namespace JSL::Interface;
        class OpinionsSettings : public Aggregator<OpinionsSettings>
        {
          public:
            std::string Halberd;

            OpinionsSettings()
            {
                Name = "Opinions";
                HelpData.Commands["null"] = "This won't show up because we don't look at nested commands";
            }
            template <typename Visitor>
            void FieldList(Visitor &&v)
            {
                v(Field<std::string>{Halberd, "Halberd", {"ha", "halberd"}, "They suck!", "Tells us what you think of halberds"});
            }
        };

        class Settings : public JSL::Interface::Aggregator<Settings>
        {
          public:
            Settings()
            {
                Name = "Settings";
                HelpData.Commands["pdf"] = "Activates the pdf-output module";
                HelpData.Commands["watch"] = "Acivates asynchronous file watch mode";
            }
            int DispatchDelay;
            bool Verbose = false;
            std::vector<std::string> WatchedPatterns = {"*.tex", "*.dat"};
            OpinionsSettings Opinions;

            template <typename Visitor>
            void FieldList(Visitor &&v)
            {
                v(JSL_Field(Verbose, {"v"}, false, "Enable verbose output"));
                v(JSL_Field(DispatchDelay, {"delay"}, 10, "Dispatch delay in ms"));
                v(Opinions);
                v(JSL_Field(WatchedPatterns, {"watch"}, std::vector<std::string>({"*.tex", "*.dat"}), "Patterns to watch"));
            }
        };

        int main(int argc, char **argv)
        {
            JSL::Log::Global().Level = DEBUG;

            Settings S;
            S.Parse(argc, argv);
            if (S.Verbose)
            {
                LOG(INFO) << S.Commands << " Our opinion on halberds: " << S.Opinions.Halberd;
            }
            else
            {
                LOG(INFO) << "Sssh, you need to wait " << S.DispatchDelay << " seconds before asking again";
            }
        }

    With the addition of just a handful of lines (the HelpData calls, and the FieldList declarations), standard POD classes have been converted into fully-configurable entities, with automated help generation:

    .. rst-class:: terminal-block
    .. code-block:: text

        $ ./example -v -halberd They\'re Cool!
        [INFO]  [] Our opinion on halberds: They're Cool!                                         | (example.cpp: 54)
         
        $ ./example -delay 100
        [INFO]  Sssh, you need to wait 100 seconds before asking again                            | (example.cpp: 58)
         
        $ ./example -h
        Usage:
            ./example cmd1 cmd2... -flag -key value cmd3
        Keys are be indicated by at least one dash.
        Flags are booleans implicitly set to `true'.
        Commands are any space-delimited arguments not assigned to a key.
        Commands
        help            Activates the help display, then exits (equivalent to -h)

        pdf             Activates the pdf-output module

        watch           Acivates asynchronous file watch mode

        Settings
        -config         (Inbuilt)           If a file is passed to this argument, it is used to configure the remaining
                        <string>            parameters, at a lower priority than values set from the command line
                                            [Default: -none-]
        -config-delim   (Inbuilt)           This value is used to determine the delimiter used to separate key-value pairs in
                        <string>            config files.
                                            [Default: ' ']
        -delay          DispatchDelay       Dispatch delay in ms
                        <int>               [Default: 10]
        -h, -help       (Inbuilt)           If true, shows the help screen, then quits
                        <bool>              [Default: false]
        -v              Verbose             Enable verbose output
                        <bool>              [Default: false]
        -watch          WatchedPatterns     Patterns to watch
                        <vector<string>>    [Default: [*.tex, *.dat]]
        Opinions
        -ha, -halberd   Halberd            Tells us what you think of halberds
                        <string>           [Default: They suck!]

The Aggregator Class-Pattern
--------------------------------


.. jsl-class:: JSL::Interface::Aggregator
   :file: Interface/Aggregator.h

Helpers
+++++++++
 
.. toctree::
    :maxdepth: 1

    field 
    help
 
How to Build an Aggregator
----------------------------


The Aggregator Class should only be used as a base class from which a user-defined class is derived. The following steps convert an existing holder-of-variables into a configurable aggregate:

1. The starting point for an Aggregator-Class is a Plain-Old-Data object which contains a number of non-const members:

    .. code-block:: cpp

       class Data
       {
            double Member1;
            std::string Member2;
            (...)
            std::vector<std::tuple<int,int,char>> MemberN;
       };
        
2. The class must then be set to derive from a CRTP-Aggregator instance
    
    .. code-block:: cpp
        
        class Data : public JSL::Interface::Aggregator<Data>
        {
            (...)
        }

3. A new member function must be added (either public or private). Because it is a template function, it must reside fully in the header file.

    .. code-block:: cpp
     
        template<typename t> 
        void FieldList(t &&vt)
        {
         
        }

4. For each member which will be configurable, add an entry into the field list of the associated :ref:`Field object<field>`. This sets the display names and documentation, but also establishes the Aliases and Default values asigned to the member.

    .. code-block:: cpp
        
        template<typename t> 
        void FieldList(t &&vt)
        {
            //fields can often infer the type
            t(Field(Member1, "Double Member", {"f","mem1","alias"}, 0, "This is documentation explaining what member 1 is"));

            //But strings can be a pain, so explicitly name it
            t(Field<std::string>(Member2,"String Member", {"mem2"}, "", "Documentation for member 2");

            (...)

            //Macro interface automatically stringiifies 'MemberN' if a custom name is not needed 
            t(JSL_Field(MemberN, {"memN"}, {}, "Docs for this member"));
        }
  
5. Aggregators can be nested automatically, by passing them in the FieldList
    
    .. code-block:: cpp
     
        class Nested :  public JSL::Interface::Aggregator<Data>
        {
            //nested members and a FieldList enumerating them
            (...)
        }

        
        class Data : public JSL::Interface::Aggregator<Data>
        {
            Nested nested;
             
            (...)
             
            template<typename t> 
            void FieldList(t &&vt)
            {
                t(nested); // no docs needed, just connect the heirarchy

                (...)
            }
        }
6. It is then simple to instantiate the object, and then parse it at run time:

    .. code-block:: cpp
        
        // assume classes declared or included 
        int main(int argc, char **argv)
        {
            Data d;
            d.Parse(argc,argv);

            //members are configured and can be used as normal
            
        }
7. Default constructors can be used to provide contextual information which appears when the Help Menu is activated

    .. code-block:: cpp
     
        class Nested :  public JSL::Interface::Aggregator<Data>
        {
            Nested()
            {
                Name = "Nested Settings";
            }
             
            (...)
        }
        class Data :  public JSL::Interface::Aggregator<Data>
        {
            Data()
            {
                Name = "Data Object";
                HelpData.Commands["push"] = "Pushes the data to the server";
                (...)
            }
        }
       

    Then when :ref:`Help is requested<help>`, the output is:

    .. rst-class:: terminal-block
    .. code-block:: text
        
        $ ./example -h
            Usage:
                ./example cmd1 cmd2... -flag -key value cmd3
            ----- Commands -----
            push         Pushes the data to the server
             
            ----- Data Object ------
            -f, -mem1,   Double Member   This is documentation explaining
            -alias       <double>        what member 1 is
            
            (...)

            ----- Nested Settings -----
            (...)



