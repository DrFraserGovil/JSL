#pragma once
#include <JSL/IO/Filesystem.h>
#include <JSL/IO/Vault/VaultHeaders.h>
#include <JSL/Strings/ParseTo.h>
#include <climits>
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

			/*! @brief Parse the datafile and convert each line into a tuple of datatypes based on an assumed regular tabular. Then performs a callback function on each line.
				@details Each line is converted into a tuple of types specified by the ColumnTypes parameter (i.e. ForConvertedLineIn<int,int,int> attempts to read the file as a set of three integers). std::apply is then used to unpack the tuple, and supply it as arguments to the function
				@tparam ColumnTypes An (arbitrary) number of typenames, representing the converted types of each element. typenames must have an associated String::ParseTo() function.
				@tparam Funct (optional -- usually compiler-inferred) template parameter for the type of the perline callback function.
				@param delimiter The string delimiter between ColumnTypes in the file
				@param perLineFunction The function to be called on each line.
				@throws runtime_error If the columns of the file cannot be interpreted as a regular grid of len(ColumnTypes) with consistent data types with the specified delimiter
				@throws runtime_error If a datatype in a column cannot be converted into the specified type
			*/
			template <typename... ColumnTypes, typename Func>
				requires std::invocable<Func, ColumnTypes...>
			void ForConvertedLineIn(std::string_view delimiter, Func perLineFunction)
			{
				ForLineIn([&](std::string_view line)
					{	
						auto row = JSL::String::ParseTo<std::tuple<ColumnTypes...>>(line,delimiter);
						std::apply(perLineFunction,row); });
			}

			/*! @brief Read the file into memory, whilst parsing each line into a tuple of datatypes based on an assumed regular type scheme.
				@details Each line is converted into a tuple of types specified by the ColumnTypes parameter (i.e. ForConvertedLineIn<int,int,int> attempts to read the file as a set of three integers).
				@tparam ColumnTypes An (arbitrary) number of typenames, representing the converted types of each element. typenames must have an associated convert() function.
				@param delimiter The string delimiter between ColumnTypes in the file
				@throws runtime_error If the columns of the file cannot be interpreted as a regular grid of len(ColumnTypes) with consistent data types with the specified delimiter
				@throws runtime_error If a datatype in a column cannot be converted into the specified type
				@returns A vector of tuples representing the parsed data
			*/
			template <typename... ColumnTypes>
			std::vector<std::tuple<ColumnTypes...>> AsTable(std::string_view delimiter = " ")
			{
				std::vector<std::tuple<ColumnTypes...>> rows;
				ForLineIn([&](std::string_view line)
					{	
						auto row = JSL::String::ParseTo<std::tuple<ColumnTypes...>>(line,delimiter);
						rows.push_back(row);
					});
				return rows;
			}

			/*! @brief Read the file into memory, whilst parsing each line into the specified datatype
				@details Each line is converted into the chosen type
				@tparam T the type to convert each line into. Each line must be wholly convertible into this type, which may be of any type supported by String::ParseTo 
				@param stringNotInInput This value can be changed to arbitrary strings to prevent the internal tuple parser breaking up input lines of text
				@throws runtime_error If a line cannot be converted into the specified type
				@returns A vector of type T representing the parsed data
			*/
			template <typename T>
			std::vector<T> AsConvertedLines(std::string_view stringNotInInput = " ")
			{
				std::vector<T> out;
				ForConvertedLineIn<T>(stringNotInInput,[&out](T a)
					{ out.push_back(a); });
				return out;
			}
		};

		std::map<std::string, Stream> FileIndex;
		std::ifstream VaultStream;

		/// @brief Scans the archive looking for header blocks, and extracting the metadata to build an index of the vault contents
		void BuildIndex();
		std::string Name;
		bool Initialised = false;
		Policy Strictness = Policy::Strict;

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
		/// @details Equivalent to calling [file].AsLines()
		/// @param file The file in the archive to be acted upon
		/// @throw runtime_error if file is not within the vault
		/// @return A vector with element i being the i'th line of the file (excluding the newline character itself)
		std::vector<std::string> AsLines(const std::string &file);

		/*!
			@brief Parse the datafile and convert each line into a tuple of datatypes based on an assumed regular tabular. Then performs a callback function on each line.
			@details Equivalent to calling [file].ForConvertedLineIn(...)
			@tparam ColumnTypes An (arbitrary) number of typenames, representing the converted types of each element. typenames must have an associated convert() function.
			@tparam TupleFunctor (optional -- usually compiler-inferred) template parameter for the type of the per-tuple callback function.
			@param file The file in the archive to be acted upon
			@param delimiter The string delimiter between ColumnTypes in the file
			@param perLineFunction The function to be called on each line.
			@throws runtime_error If the columns of the file cannot be interpreted as a regular grid of len(ColumnTypes) with consistent data types with the specified delimiter
			@throw runtime_error if file is not within the vault
			@throws runtime_error If a datatype in a column cannot be converted into the specified typr
			@throws logic_error when called whilst in write mode
			@throws logic_error when fileName not in the archive
		*/
		template <typename... ColumnTypes, typename Func>
		void ForConvertedLineIn(const std::string &file, std::string_view delimiter, Func perLineFunction)
			requires std::invocable<Func, ColumnTypes...>
		{
			operator[](file).ForConvertedLineIn<ColumnTypes...>(delimiter, perLineFunction);
		}

		/*!
			@brief Read the specified file into memory, whilst parsing each line into a tuple of datatypes based on an assumed regular type scheme.
			@tparam ColumnTypes An (arbitrary) number of typenames, representing the converted types of each element. typenames must have an associated convert() function.
			@param file The file in the archive to be acted upon
			@param delimiter The string delimiter between ColumnTypes in the file
			@throws runtime_error If the columns of the file cannot be interpreted as a regular grid of len(ColumnTypes) with consistent data types with the specified delimiter
			@throw runtime_error if file is not within the vault
			@throws runtime_error If a datatype in a column cannot be converted into the specified type
			@returns A vector of tuples representing the parsed data
		*/
		template <typename... ColumnTypes>
		std::vector<std::tuple<ColumnTypes...>> AsTable(const std::string &file, std::string_view delimiter)
		{
			return operator[](file).AsTable<ColumnTypes...>(delimiter);
		}
	};
} // namespace JSL::IO