/*
    This code is based on some work by Alberto Lepe (dev@alepe.com), heavily modified by JFG

    I have taken the view that Logging -- when active -- should never be performance-critical code, and so some of this is written in an idiomatic fashion, rather than an optimised one. 
*/

#pragma once
#include <unistd.h> // For isatty()
#include <cstdio>   // For fileno() and stderr
#include <iostream>
#include <sstream>
#include <string_view>
#include "Log_Utils.h"
#include "ANSI_Codes.h"
#include "../Strings/split.h"
/*!
    @brief The main log interface. Pipe output to it as you would std::cout.
    
    @details LOG is a specialised macro-interface to the LoggerCore object.  If the level check evaluates to false, then the <<'d inputs are completely ignored and are not executed, useful for skipping `expensive' operations during a ::DEBUG run. Defined in @fileinfo{path}

    @param level A ::LogLevel object, if greater than the LogConfig::Level value, nothing happens (and the expansion is ignored) 
    @returns If the level is suitable, a temporary LoggerCore object, which functions as a specialised Stream object, accepting values passed via '<<'. Otherwise, does nothing, and does not evaluate any subsequent pipe commands
*/
#define LOG(level) \
    if (!(level <= JSL::Log::Config.Level)) {} \
    else (JSL::Log::LoggerCore(level,__LINE__,__func__,__FILE__))

// Defines the externally-linked variables; has to be placed at file-scope 
#define Initialise_JSL_Log()\
    JSL::Log::ConfigObject JSL::Log::Config; \
    std::mutex JSL::Log::StreamMutex; \
    std::vector<size_t> JSL::Log::PreviousLines= std::vector<size_t>(LogLevel::MAXLEVEL,0);


namespace JSL::Log
{
    //! The global ConfigObject used by @ref LOG and LoggerCore to determine what messages are printed, and any associated formatting 
	extern ConfigObject Config;

	//! Used to prevent log-interleaving, and make the LOG (mostly) thread-safe
	extern std::mutex StreamMutex;

	//! A tracker of the number of lines printed since each type of ::LogLevel. Used for LoggerCore::ErasePrevious()
	extern std::vector<size_t> PreviousLines;

    /*!
        @brief Created during a \ref LOG call as a temporary object, and acts as a custom output stream

        @details Each LoggerCore object is associated with a single 'log entry'.
    */
    class LoggerCore
    {
        public:
            /*!
                @brief Constructor for the LoggerCore, initialises an entry in the output log. Should never be called outside \ref LOG.

                @details Note that the Level is used purely for formatting purposes: a LoggerCore object *always* outputs a stream. The `log suppression checks' are performed by \ref LOG. The three 'calling' parameters are used to locate the origin of a LOG; they are provided by compile-time flags, but are only used by ERROR and WARN output. 

                @param level The LogLevel that the associated entry will be on. Determines formatting.
                @param callingLine This is the ƒile line on which the \ref LOG call appears
                @param callingFunction The function in which the LOG call occurred. Provides a pseudo-stack-unwinding.
                @param callingFile The file in which the callingFunction is found.
            */
            LoggerCore(LogLevel level,int callingLine,const std::string & callingFunction,std::string callingFile)
            {
                StreamActive = false;
                Level = level;
                Insert = "";
                if (Level <= 1)
                {
                    Insert = "Line " + std::to_string(callingLine) + " of " + callingFile + " in function " + callingFunction ;
                    Insert += "\n";
                }
            }

            /*!
                @brief Custom destructor which performs the actual output to the terminal
                
                @details Under normal usage, the LOG(LEVEL) call will cause the temporary object to almost instantly go out of scope. Therefore this destructor will usually be called as soon as the last element is streamed into the object, however there may sometimes be a delay.
            */
            ~LoggerCore()
            {
                if (StreamActive) //only add the output to stream if "<<" was actually called
                {
                    endMessage();
                }
            }

            /*!
                @brief The streaming operator, allowing data to be piped into the stream just as if it were std::cout

                @param msg The data to be added to the stream. Must be convertible to a stringstream object
                @returns A reference to the object, allowing for 'chaining' the stream; stream << a << b << c
            */
            template<class T>
            LoggerCore &operator<<(const T &msg)
            {
                if (!StreamActive)
                {
                    StreamActive = true;
                    Header();
                    Buffer << Insert;
                }
                Buffer << msg;
                return *this;
            } 

            /*!
                @brief Deletes the specified number of lines from the terminal

                @details Uses ANSI escape codes to clear lines and reset the cursor to emulate 'deleting' lines. See <a href="../../html/utility/log_erase.html">Erasing Logs Documentation</a> for more information.

                @param nLines The number of lines (counted by line breaks, not word wrapping) to be deleted
            */
            void Erase(int nLines)
            {
                if (Log::Config.TerminalOutput)
                {	
                    for (int i = 0; i < nLines; ++i)
                    {
                        std::cout << Cursor::CursorUp << Cursor::ClearLine;
                    }
                    std::cout << std::flush;//only do because deletion is expected to be 'instant', not buffered
                    for (int i = 0; i < LogLevel::MAXLEVEL;++i)
                    {
                        int n = Log::PreviousLines[i];
                        Log::PreviousLines[i] = std::max(0,n-nLines);
                    }
                }
            }

            /*!
                @brief Deletes log entries until it has deleted an entry equal to the Level, or it encounters a higher level entry, which is not deleted

                @details This function is designed to delete previously added lines, but is respectful of those which are higher priority (and disrespectful of lower-priority lines).See <a href="../../html/utility/log_erase.html">Erasing Logs Documentation</a> for more information.
            */
            void ErasePrevious()
            {
                std::unique_lock<std::mutex> lock(Log::StreamMutex);
                size_t erase = Log::PreviousLines[Level];
                size_t block = 0;
                for (int i = 0; i < Level; ++i)
                {
                    size_t pli = PreviousLines[i];
                    if (pli < erase && pli > 0 && (block == 0 || pli < block))
                    {
                        block = PreviousLines[i];
                    }
                }
                size_t safe = 0;
                if (block > 0)
                {
                    for (int i = Level +1; i < LogLevel::MAXLEVEL; ++i)
                    {
                        size_t pli = PreviousLines[i];
                        if (pli > safe && pli < block && pli > 0 )
                        {
                            safe = pli;
                        }
                    }
                    erase = safe;
                }

                Erase(erase);
            }
        
        private:
            //! The internal Buffer to which LoggerCore::operator<< is streamed, and which is then output to terminal.
            std::stringstream Buffer;

            //! The ::LogLevel of the log entry associated with this object. Used only to determine formatting.
            LogLevel Level;

            //! An internal flag which checks if anything was actually streamed to the object. If this is true, then the destructor calls endMessage() and outputs the Buffer to stream. 
            bool StreamActive;
            
            //! A string which holds the callingLine/Function/File data after the constructor is called, but before the stream is activated.
            std::string Insert;

            //! If Config.UseHeaders is true, this function generates headers (such as [ERROR]) for the log entry. 
            void Header()
            {
                std::string label;
                std::string fmt;
                switch(Level) {
                    case DEBUG: fmt = JSL::Text::Blue;label = "[DEBUG] "; break;
                    case INFO: fmt=JSL::Text::White;label = "[INFO]  "; break;
                    case WARN: fmt=JSL::Text::Purple;label = "[WARN]  "; break;
                    case ERROR: fmt=JSL::Text::Red;label = "[ERROR] "; break;
                    default: throw std::runtime_error("Invalid logger argument");
                } 
                if (Config.TerminalOutput)
                {
                    Buffer << fmt;
                }
                if (Config.ShowHeaders)
                {
                    Buffer << label;
                }
            }
            
            /*!
                @brief The point at which the Buffer is added to the output stream
                
                @details Called by the destructor if the Buffer contains data. This function tidies up the buffer and formats it for output, before adding it to the output stream.
            */
            void endMessage()
            {
                if (Config.TerminalOutput)
                {
                     Buffer <<Text::Reset; //reset the font colors for all subsequent data
                }


                //now format the data so that linebreaks are suitably indented
                std::string linebreak = "\n";
                if (Config.ShowHeaders)
                {
                    linebreak += "\t";
                }
                auto message = split(Buffer.view(),"\n");
                std::string buffer = "";

                //iterate across all lines in the entry 
                {
                    std::unique_lock<std::mutex> lock(Log::StreamMutex); //lock the stream to prevent interleaving

                    //the first line automatically includes the correct indentation -- the header accounts for that
                    std::cout << message[0]; 

                    //subequent lines need to indent (or not) based on the presence of the header.
                    for (size_t i = 1; i < message.size(); ++i)
                    {
                        std::cout << linebreak << message[i];
                    }

                    if (Config.AppendNewline)
                    {
                        std::cout << "\n"; 
                    }

                    //save the data to the 'erase' memory banks
                    auto nlines = message.size();
                    Log::PreviousLines[Level] =0;
                    for (int i = 0; i < LogLevel::MAXLEVEL; ++i)
                    { 
                        Log::PreviousLines[i] += nlines; //do this inside the mutex so line ordering is correct
                    }
                }
            }
    };
}
