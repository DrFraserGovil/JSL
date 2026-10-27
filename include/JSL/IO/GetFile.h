#pragma once
#include <JSL/IO/ForLineIn.h>

namespace JSL::IO
{

	/*! @brief Reads a text file into memory, saving it as a raw string
		@param fileName the file to be read into the string
		@returns A string representing the full file contents (including any escape characters such as newlines)
		@throws runtime_error if the file cannot be located
	*/
	std::string getFile(const std::filesystem::path fileName);

	/*!
	 * @brief Reads a file into memory, saving each line as an element of a vector
	 * @param fileName The file to be read
	 * @return A vector where each element is a line of text from the original (newline characters are not included). The length of the vector is equal to the number of lines in the file
	 * @throws runtime_error if the file cannot be located or opened.
	 */
	std::vector<std::string> getFileLines(const std::filesystem::path fileName);

	/*!
	 * @brief Reads up to the first `nLines` lines of file into memory, saving each line as an element of a vector
	 * @detail The returned vector has `size() < nLines` if the file is shorter than `nLines`.
	 * @param fileName The file to be read
	 * @param nLines The maximum number of lines to read
	 * @param includeBlank If true, whitespace-only lines are counted. Otherwise they are skipped and do not count towards nLines.
	 * @return A vector where each element is a line of text from the original (newline characters are not included). The length of the vector is equal to the number of lines in the file
	 * @throws runtime_error if the file cannot be located or opened.
	 */
	std::vector<std::string> getFileLines(const std::filesystem::path fileName, size_t nLines, bool includeBlank = true);

	/*!
	 * @brief Reads a file into memory, splitting the lines into chunks determined by a delimiter
	 * @param fileName The file to be read
	 * @param delimiter The character sequence used to separate words (does not appear in output)
	 * @return A vector-of-vectors, where each element in the outer layer is a line from the file (no. of elements = no .of lines), and the elements in the inner layer are the 'words' (no. of elements = no. of words)
	 * @throws runtime_error if the file cannot be located or opened.
	 */
	std::vector<std::vector<std::string>> getFileWords(const std::filesystem::path fileName, std::string_view delimiter);

} // namespace JSL::IO
