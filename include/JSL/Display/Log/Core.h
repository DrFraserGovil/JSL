/*
	This code is based on some work by Alberto Lepe (dev@alepe.com), heavily modified by JFG

	I have taken the view that Logging -- when active -- should never be performance-critical code, and so some of this is written in an idiomatic fashion, rather than an optimised one. 
*/
#pragma once

#include <JSL/Display/Log/Config.h>
#include <JSL/Strings/Manipulate.h>

/*!
	@brief The main log interface. Pipe output to it as you would std::cout.
	
	@details LOG is a specialised macro-interface to the Core object.  If the level check evaluates to false, then the <<'d inputs are completely ignored and are not executed, useful for skipping `expensive' operations during a ::DEBUG run. Defined in @fileinfo{path}

	@param level A ::LogLevel object, if greater than the LogConfig::Level value, nothing happens (and the expansion is ignored) 
	@returns If the level is suitable, a temporary Core object, which functions as a specialised Stream object, accepting values passed via '<<'. Otherwise, does nothing, and does not evaluate any subsequent pipe commands
*/
#define LOG(level) \
	if (!(level <= JSL::Log::Global::Config.Level)) {} \
	else (JSL::Log::Core(level,__LINE__,__func__,__FILE__))


namespace JSL::Log
{
	

	/*!
		@brief Created during a \ref LOG call as a temporary object, and acts as a custom output stream

		@details Each Log::Core object is associated with a single 'log entry'.
	*/
	class Core
	{
		public:
			/*!
				@brief Constructor for the Core, initialises an entry in the output log. Should never be called outside \ref LOG.

				@details Note that the Level is used purely for formatting purposes: a Core object *always* outputs a stream. The `log suppression checks' are performed by \ref LOG. The three 'calling' parameters are used to locate the origin of a LOG; they are provided by compile-time flags, but are only used by ERROR and WARN output. 

				@param level The LogLevel that the associated entry will be on. Determines formatting.
				@param callingLine This is the ƒile line on which the \ref LOG call appears
				@param callingFunction The function in which the LOG call occurred. Provides a pseudo-stack-unwinding.
				@param callingFile The file in which the callingFunction is found.
			*/
			Core(LogLevel level,int callingLine,const std::string & callingFunction,std::string callingFile);


			/*!
				@brief Custom destructor which performs the actual output to the terminal
				
				@details Under normal usage, the LOG(LEVEL) call will cause the temporary object to almost instantly go out of scope. Therefore this destructor will usually be called as soon as the last element is streamed into the object, however there may sometimes be a delay.
			*/
			~Core();

			/*!
				@brief The streaming operator, allowing data to be piped into the stream just as if it were std::cout

				@param msg The data to be added to the stream. Must be convertible to a stringstream object
				@returns A reference to the object, allowing for 'chaining' the stream; stream << a << b << c
			*/
			template<class T>
			Core &operator<<(const T &msg)
			{
				if (!StreamActive) //lazy opening of the steam
				{
					StreamActive = true;
					Header();
					Buffer << Insert;
				}
				Buffer << msg;
				return *this;
			} 

			/*
				Enables the logger to have a persistent and selective formatting options
			*/
			Core &operator<<(Format::Command format);
			Core &operator<<(Format::FormatGroup group);

			Format::FormatGroup CurrentFormat;
			
		
		private:
			//! The internal Buffer to which Core::operator<< is streamed, and which is then output to terminal.
			std::stringstream Buffer;
			std::stringstream BufferPreamble;
			bool ManualFormat = false;
			//! The ::LogLevel of the log entry associated with this object. Used only to determine formatting.
			LogLevel Level;

			//! An internal flag which checks if anything was actually streamed to the object. If this is true, then the destructor calls endMessage() and outputs the Buffer to stream. 
			bool StreamActive;
			
			//! A string which holds the callingLine/Function/File data after the constructor is called, but before the stream is activated.
			std::string Insert;

			std::string LineSuffix = "";
			std::string FirstLineSuffix = "";

			//! If Config.UseHeaders is true, this function generates headers (such as [ERROR]) for the log entry. 
			void Header();
			
			/*!
				@brief The point at which the Buffer is added to the output stream
				
				@details Called by the destructor if the Buffer contains data. This function tidies up the buffer and formats it for output, before adding it to the output stream.
			*/
			void EndMessage();
		
			
	};
}
