#pragma once
#include "string.h"
#include <fstream>
namespace JSL
{	
	/*! 
	 * Creates a blank file at the specified location, overwriting any other file at the given location
	 * \param filename The file which the system will attempt to open
	*/
	void initialiseFile(const std::string & filename)
	{
		std::fstream file;
		file.open(filename,std::ios::out);	
		file.close();
	}


	/*!
	 * Opens the provided file and appends the provided string to the file, before closing it. If the file does not exist, it creates it. 
	 * \param filename The target file location
	 * \param content The desired string to be appended to the file (accepts control characters)
	*/
	void writeStringToFile(const std::string & filename, const std::string & content)
	{
		std::fstream file;
		file.open(filename, std::ios::app);
		file << content;
		file.close();
	}
	
	/*!
	 * As with writeStringToFile, but accepts a vector of templated entities. The writing loops over the vector and writes them one at a time, separated by the delimiter object
	 * \param filename The target file location
	 * \param contentVector A vector of templated objects to be written to file
	 * \param delimiter The character(s) to be written after every entry of contentVector *except* the final entry
	 * \param includeTerminalLineBreak If true, appends a linebreak character at the end of the vector. Useful for sequentially writing rows of data to file.
	*/
	template<class T>
	void writeVectorToFile(const std::string & filename, const std::vector<T> & contentVector, const std::string & delimiter, bool includeTerminalLineBreak)
	{
		std::fstream file;
		file.open(filename,std::ios::app);
		
		for (int i = 0; i < contentVector.size(); ++i)
		{
			file << contentVector[i];
			if (i < contentVector.size() - 1)
			{
				 file << delimiter;
			 }
		}	
		
		if (includeTerminalLineBreak)
		{
			file << "\n";
		}
		file.close();
	} 
	
	/*!
	 * As with writeStringToFile and writeVectorToFile, but accepts a vector<vector> of templated entities. The writing loops over the outer vector (the rows), and then at each step, the inner vectors(the columns). Writing them one at a time, separated by the delimiter objects. Objects need not be square matrices to be successfully written. 
	 * \param filename The target file location
	 * \param contentMatrix A vector<vector> of templated objects to be written to file
	 * \param columnDelimiter The character(s) to be written after every individual entry *except* the final entry in each row
	 * \param rowRelimiter The character(s) to be written at the end of each row *including* the final row. This will probably be a linebreak!
	*/
	template<class T>
	void writeMatrixToFile(const std::string & filename, const std::vector<std::vector<T>> contentMatrix, const std::string & columnDelimiter, const std::string & rowDelimiter)
	{
		std::fstream file;
		file.open(filename,std::ios::app);
		
		for (std::vector<T> contentRow : contentMatrix)
		{
			for (int i = 0; i < contentRow.size(); ++i)
			{
				file << contentRow[i];
				if (i < contentRow.size() - 1)
				{
					 file << columnDelimiter;
				 }
			}		
			file << rowDelimiter;
		}	
		file.close();
	}
}
