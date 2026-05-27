#pragma once
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <string_view>
#include <ranges>
#include <JSL/FileIO/TemplateWriters.h>
namespace JSL::IO
{

    /*! 
	 * Creates a blank file at the specified location, overwriting any other file at the given location
	 * \param filename The file which the system will attempt to create
	*/
    void initialise(const std::filesystem::path& filename);

    /*!
        @brief A basic wrapper around the ofstream constructor that performs a check to ensure that the file is opened
        @param filename The file to be opened
        @param mode The open-mode; defualts to out (existing file overwritten by new content)
        @throw logic_error If the file stream is not open
        @returns A filestream pointing towards the requested file
    */
    std::ofstream openStream(const std::filesystem::path & filename, std::ios_base::openmode mode = std::ios::out);

    /*!
	 * @brief Opens the provided file and writes the provided string to the file, before closing it. If the file does not exist, it creates it. 
	 * \param filename The target file location
	 * \param content The desired string to be written to the file (accepts control characters)
     * \param mode The open-mode; defaults to out (existing file overwritten by new content)
	*/
	void writeString(const std::filesystem::path& filename, std::string_view content,std::ios_base::openmode mode = std::ios::out);


    





}