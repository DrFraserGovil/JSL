#pragma once
#include <cmath>

namespace JSL::Terminal
{
	/*!
	 * @brief A class representing the current state of the terminal 
	 */
	class Environment
	{
		public:
			//! If true, Rows() and Columns() auto-update every time they are queried. If false, they update only on construction and when Cache() calls are made
			bool DynamicUpdates = false;
			//! The number of vertical lines in the current terminal (without scrolling)
			size_t Rows();
			//! The number of horizontal characters per line in the current terminal
			size_t Columns();
			//! The size of a `tab block' in the current terminal
			size_t TabSize();
			//! @brief Computes the size of a tabstop and caches it for future use
			//! @details by inserting a tab, requesting a Device Status Report to find the current column, then deleting the line.
 			void CacheTabs();
			//! Uses ioctl to query the current dimensions of the terminal, and cache them for future retrieval
 			void CacheSize(); 

			void CacheANSI();
			/*! @brief  Determine if the output stream can interpret ANSI escape sequences
				@details Uses isatty (and _isatty for windows) to determine if stdout can render ANSI commands.
				@returns True if the output is capable of interpreting format commands (i.e. a terminal)
				False if the terminal is plaintext (i.e. a file)
			*/
			bool IsANSICapable();
			// Declared friend so singleton can construct 
			friend Environment & Global();
		private:
			//! Private constructor for singleton uniqueness
		 	Environment();
			//! Internal row cache
			size_t _Rows;
			//! Internal column cache
			size_t _Columns;
			//! Internal tab cache
			size_t _Tabsize;

			//! ansi state
			bool AnsiActive;

			//! Default destructor; only here for Ro5 purposes really! 
			~Environment() = default;
			//! Delete to ensure singleton uniqueness
			Environment(const Environment &) = delete;
			//! Delete to ensure singleton uniqueness
			Environment & operator=(const Environment &) = delete;
			//! Delete to ensure singleton uniqueness
			Environment(Environment &&) = delete;
			//! Delete to ensure singleton uniqueness
			Environment & operator=(Environment &&) = delete;
	};

	/*!
	 * @brief Access the global terminal environment state
	 * @return The singleton-instance of the Terminal::Environment
	 */
	Environment & Global();
};