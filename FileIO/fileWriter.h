#pragma once
#include "string.h"
#include <fstream>
#include <iostream>
#include "../System/System.h"
namespace JSL
{	
	/*! 
	 * Creates a blank file at the specified location, overwriting any other file at the given location
	 * \param filename The file which the system will attempt to open
	*/
	void inline initialiseFile(const std::string & filename)
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
	void inline writeStringToFile(const std::string & filename, const std::string & content)
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
	void inline writeVectorToFile(const std::string & filename, const std::vector<T> & contentVector, const std::string & delimiter, bool includeTerminalLineBreak)
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
	
	namespace internal
	{
		/*!
			* This is a helper function for writeMultiVectorToFile, and performs the recursive looping over the variadic vector templates.
		*/
		template<typename T, typename... Ts>
		void inline variadicVectorPrint(std::fstream& os, const std::string & delimiter, int i, const std::vector<T>& first, const std::vector<Ts>&... args)
		{
			os << std::setprecision(10) << first[i];
			if constexpr (sizeof...(args) > 0)
			{
				os  <<delimiter;
				variadicVectorPrint(os,delimiter,i,args...); // shunts the remaining args one element over, so the second element becomes "first", and hence recursively loops until no elements left in args
			}
			else
			{
				os << "\n";
			}
		}

		template<typename T, typename...Ts>
		bool inline variadicLengthEquality(size_t length, const std::vector<T> & first, const std::vector<Ts>&...args)
		{
			if constexpr (sizeof...(args) > 0)
			{
				
				return (first.size() == length) &&  variadicLengthEquality(length,args...);
			}
			else
			{
				return first.size() == length;
			}
		}
	}
	/*!
	 * As with writeVectorToFile, but accepts arbitrary vectors of templated entities. The writing loops over the length of the vectors (which must all be the same length), and writes them sequentially, separated by the delimiter, and a linebreak at the end of each row - i.e v1[0], v2[0], v3[0], ... (linebreak) v1[1], v2[1], ... etc.
	 * \param filename The target file location
	 * \param delimiter The character(s) to be written after every individual vecs entry *except* the final entry on each row, which is a linebreak.
	 * \param v1 The first vector to be written to file
	 * \param vecs A variadic template for any number (including 0) of additional vectors, of any type (said type must have support for the streaming operator, <<). All vecs must be the same length
	*/
	template<typename T, typename... Ts>
	void inline writeMultiVectorToFile(const std::string & filename, const std::string & delimiter, const std::vector<T>& v1, const std::vector<Ts>&...  vecs)
	{
		
		if  constexpr (sizeof...(vecs) > 0)
		{	
			bool allEqual = internal::variadicLengthEquality(v1.size(),vecs...);
			Assert("All vectors in writeMultiVectorToFile should be of the same length",allEqual);
		}
		int N = v1.size();
		std::fstream file;
		file.open(filename,std::ios::app);
		for (int i = 0; i < N; ++i)
		{
			internal::variadicVectorPrint(file,delimiter,i,v1,vecs...);			
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
	void inline writeMatrixToFile(const std::string & filename, const std::vector<std::vector<T>> contentMatrix, const std::string & columnDelimiter, const std::string & rowDelimiter)
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


	/*!
	 * A funciton similar to writeMatrix, but customised to the dumb format required for gnuplot heatmaps with their additional weird linebreak everytime the x-value changes
	*/
	template<class T, class R, class S>
	void inline writeHeatMapToFile(const std::string & filename, const std::vector<R> & x, const std::vector<S> & y, const std::vector<std::vector<T>> z, const std::string & columnDelimiter)
	{
		std::fstream file;
		file.open(filename,std::ios::app);

		for (int i =0 ; i < x.size(); ++i)
		{
			for (int j = 0; j < y.size(); ++j)
			{
				file << x[i] << columnDelimiter << y[j] << columnDelimiter << z[j][i] << "\n";
			}
			file << "\n";
		}
		file.close();
	}
}
