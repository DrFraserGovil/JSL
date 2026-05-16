#pragma once

#include <string_view>
#include <vector>
#include <string>
namespace JSL::String
{
	
	/*! @brief Returns the true width of a string, taking into account tab characters and escape sequences
		@param str The string to be measured
		@param tabSize the width (in characters) of a tab block (individual tab characters may occupy less space than this)
		@warning This function makes the assumption that the tab size used by the output renderer is known at runtime, and all control sequences are of the form \x1b...m. This is a common format for terminal control sequences, but may not be universally true. Use with caution.
		@return An approximate calculation of the output size of the string, in characters
	*/
	size_t trueSize(std::string_view str,size_t tabSize = 4);

	/*!	@brief  Breaks a string up into chunks of a specified render size, breaking at whitespace
		@details Lines shorter than the width are right-padded with spaces to ensure they are meet this minimum width. Lines will only be wrapped at whitespace characters, so if a single word is longer than the width, it will be placed on a line by itself and exceed the width limit. 
		@param str The view into the string to be wrapped 
		@param width The target size of the wrapped lines
		@return A vector of strings at least ``width`` in size. Single-word lines may exceed this size.  
	*/
	std::vector<std::string> wrapToWidth(std::string_view str, size_t width);

	/*! @brief Formats a set of input strings as columns in a table, wrapping them to the column width, and inserting whitespace to ensure alignment 
		@param input A set of strings, each representing the columns of a table 
		@param widths The widths (in characters) of the 
		@param delimiter 
	 */
	std::string tableFormat(std::vector<std::string_view> input, std::vector<size_t> widths, std::string_view delimiter);
	
	/*! @brief 
		@param input 
		@param width 
		@param delimiter 
	 */
	std::string tableFormat(std::vector<std::string_view> input, size_t width, std::string_view delimiter);

}