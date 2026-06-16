#pragma once
#include <JSL/IO/Filesystem.h>
#include <JSL/IO/Vault/VaultHeaders.h>
#include <fstream>
#include <functional>
#include <map>
#include <set>
#include <string_view>
namespace JSL::IO
{
	class VaultReader
	{
	  private:
		class Stream
		{
		  private:
			/// The number of bytes in the file (as reported by the header block)
			size_t Size;

			/// Byte offset to the file's data in the archive
			std::streampos FileStartPos;

			/// A copy of the stream so it can read by itself. This is shared between all Stream instances, so is not thread safe.
			std::ifstream &StreamCopy;

		  public:
			///@brief Initialise the stream
			/// @param size The number of bytes (equal to the non-wide character count) which makes up the stream. Inferred from the header block of the Vault
			/// @param offset The position in the overall vault stream where the file begins
			/// @param stream A reference to the vault stream. This is shared between all Stream instances, so is not thread safe.
			Stream(size_t size, std::streampos offset, std::ifstream &stream);

			/// @brief Copy the contents of the file into memory
			/// @return A string representing the entire contents of the file
			std::string AsText();

			/// @brief Copy the contents of the file into memory, storing each line as an element in a vector
			/// @return A vector with element i being the i'th line of the file (excluding the newline character itself)
			std::vector<std::string> AsLines();

			/// @brief Step through the file line by line, performing a callback on each line
			/// @details Surprisingly complex internel implementation, as the individual file blocks are not guaranteed to lie on
			/// @param callback A function to be performed on each line (excluding the newline character itself) of the text in the file
			void ForLineIn(std::function<void(std::string_view)> callback);
		};
		std::map<std::string, Stream> FileIndex;
		std::ifstream VaultStream;

		/// @brief Scans the archive looking for header blocks, and extracting the metadata to build an index of the vault contents
		void BuildIndex();
		std::string Name;
		bool Initialised = false;

	  public:
		///! @brief Default constructor that leaves the Vault in an uninitialised state - Open() must be called to make the Vault functional
		VaultReader();

		/*! @brief Constructs the vault and then immediately passes the arguments to Open()
			@param filename the file to be read from
			@param policy The strictness policy applied to file operations. Policy::Strict throws errors, whilst Policy::Generous will overwrite files etc.
		*/
		VaultReader(std::string_view filename, IO::Policy policy = Policy::Strict);

		/*! @brief Initialises the vault, targeting a location which is read in as a tar-like archive
			@param filename the file to be read from
			@param policy The strictness policy applied to file operations. Policy::Strict throws errors, whilst Policy::Generous will overwrite files etc.
		*/
		void Open(std::string_view filename, IO::Policy policy = Policy::Strict);

		/// @brief Shuts down the file stream
		void Close();
		/// @brief Shuts down the file stream
		~VaultReader() noexcept;

		/// @brief Get the names of all the files in the archive
		/// @return A set of unique filenames within the archive which can be read from
		std::set<std::string> Files();

		/// @brief Access an individual 'file' within the stream
		/// @return A reference to an internal stream
		/// @throw runtime_error if file is not within the vault
		Stream &operator[](const std::string &file);

		/// @brief Perform a callback function on every line of the provided file
		/// @details Equivalent to [file].ForLineIn(callback)
		/// @param file The file in the archive to be acted upon
		/// @param callback A function which acts on each line of the file
		/// @throw runtime_error if file is not within the vault
		void ForLineIn(const std::string &file, std::function<void(std::string_view)> callback);

		/// @brief Copy the target file into memory as a string
		/// @details Equivalent to [file].AsText()
		/// @param file The file in the archive to be acted upon
		/// @throw runtime_error if file is not within the vault
		/// @return A string containing the entire file contents
		std::string AsText(const std::string &file);

		/// @brief Copy the contents of the file into memory, storing each line as an element in a vector
		/// @param file The file in the archive to be acted upon
		/// @throw runtime_error if file is not within the vault
		/// @return A vector with element i being the i'th line of the file (excluding the newline character itself)
		std::vector<std::string> AsLines(const std::string &file);
	};
} // namespace JSL::IO