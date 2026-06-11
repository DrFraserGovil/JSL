#pragma once
#include "VaultHeaders.h"
#include <fstream>
#include <memory>
#include <ostream>
#include <sstream>
#include <memory.h>

namespace JSL::IO::Vault
{
	namespace Writer
	{
		struct Exporter
		{
			internal::TarHeader headerBlock;
			std::string data;
			size_t padding;
		};

		/*!	@brief An open stream into a single 'file' within the vault.
		 */
		class BufferedStream
		{
			private:
				std::ostringstream Buffer;

			public:
				Exporter Export();
				template<class T>
				std::ostream & operator<<(const T msg)
				{
					Buffer << msg;
					return Buffer;
				}
		};

		/// @brief An alternative stream for a vault file which is too large to buffer - instead it streams directly onto disk, and post-hoc updates the metadata with the final file size.
		/// @details Only a single DirectStream can exist per-vault
		class DirectStream
		{
			private:
				std::ostream * FileStream;
				size_t StreamCount;

			public:
				DirectStream(std::ostream & diskStream);
				Exporter Export();
				template<class T>
				std::ostream & operator<<(const T msg)
				{
					//get the current position of the filestream pointer
					auto start = FileStream->tellp();
					(*FileStream) << msg;
					StreamCount += (FileStream->tellp() - start);
				}
		};

	}
}
