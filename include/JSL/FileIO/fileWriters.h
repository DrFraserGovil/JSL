#pragma once
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <string_view>
#include "../utils/jsl_error.h" 

namespace JSL
{
    /*!
     * Helper to verify file stream state.
     */
    void inline checkFile(const std::ofstream& file, const std::filesystem::path& path)
    {
        if (!file.is_open())
        {
            internal::FatalError("I/O Error") << "Could not open file: " << path.string();
        }
    }


    /*! 
	 * Creates a blank file at the specified location, overwriting any other file at the given location
	 * \param filename The file which the system will attempt to open
	*/
	void inline initialiseFile(const std::filesystem::path& filename)
    {
		std::ofstream file(filename, std::ios::out);
        checkFile(file, filename);
        file.close();
	}

    /*!
	 * Opens the provided file and appends the provided string to the file, before closing it. If the file does not exist, it creates it. 
	 * \param filename The target file location
	 * \param content The desired string to be appended to the file (accepts control characters)
     * \param mode The open-mode; defaults to append (existing file not deleted)
	*/
	void inline writeStringToFile(const std::filesystem::path& filename, std::string_view content,std::ios_base::openmode mode = std::ios::app)
    {
        std::ofstream file(filename, mode);
        checkFile(file, filename);
        file << content;
        file.close();
    }


    template<typename...Ts>
    void inline writeVectorToFile(const std::filesystem::path& filename, 
                                    const std::string& delimiter, 
                                    const std::vector<Ts>&... vecs)
    {
        static_assert(sizeof...(vecs) > 0, "At least one vector must be provided to writeVectorToFile");

        // Verify all vectors match this size using a fold expression
        const size_t rowCount = std::get<0>(std::forward_as_tuple(vecs...)).size();
        if constexpr (sizeof...(vecs) > 1)
        {
            const bool allEqual = ((vecs.size() == rowCount) && ...);
            if (!allEqual)
            {
                internal::FatalError("Vector length mismatch") << "All vectors must be same length when writing simultaneously to file";
            }
        }

        std::ofstream file(filename, std::ios::out);
        checkFile(file, filename);
        file << std::setprecision(10);

        for (size_t i = 0; i < rowCount; ++i)
        {
            bool isFirst = true;
            // Fold expression: iterates through the pack 'vecs'
            (
                (
                    file << (isFirst ? "" : delimiter) << vecs[i],
                    isFirst = false
                ), ...
            );
            file << "\n";
        }

        file.close();
    }

    /*!
	 * As with writeStringToFile and writeVectorToFile, but accepts a vector<vector> of templated entities. The writing loops over the outer vector (the rows), and then at each step, the inner vectors(the columns). Writing them one at a time, separated by the delimiter objects. Objects need not be square matrices to be successfully written. 
	 * \param filename The target file location
     * \param columnDelimiter The character(s) to be written after every individual entry *except* the final entry in each row
	 * \param contentMatrix A vector<vector> of templated objects to be written to file
	 * \param rowRelimiter The character(s) to be written at the end of each row *including* the final row. This will probably be a linebreak!
	*/
    template<class T>
    void inline writeMatrixToFile(  const std::filesystem::path& filename, 
                                    const std::string& columnDelimiter, 
                                    const std::vector<std::vector<T>>& contentMatrix,
                                    const std::string& rowDelimiter = "\n")
    {
        std::ofstream file(filename, std::ios::out);
        checkFile(file, filename);
        file << std::setprecision(10);
        for (const auto& row : contentMatrix)
        {
            for (size_t i = 0; i < row.size(); ++i)
            {
                if (i > 0) file << columnDelimiter;
                file << row[i];
            }
            file << rowDelimiter;
        }
        file.close();
    }

    /*!
	 * A function similar to writeMatrix, but customised to the dumb format required for gnuplot heatmaps with their additional weird linebreak everytime the x-value changes
	*/
	template<class T, class R, class S>
	void inline writeHeatMapToFile( const std::filesystem::path& filename, 
                                    std::string_view columnDelimiter, 
                                    const std::vector<R> & x, 
                                    const std::vector<S> & y, 
                                    const std::vector<std::vector<T>> & z)
	{
        std::ofstream file(filename, std::ios::out);
        checkFile(file, filename);
        file << std::setprecision(10);
		for (size_t i =0 ; i < x.size(); ++i)
		{
			for (size_t j = 0; j < y.size(); ++j)
			{
				file << x[i] << columnDelimiter << y[j] << columnDelimiter << z[j][i] << "\n";
			}
			file << "\n";
		}
		file.close();
	}

}