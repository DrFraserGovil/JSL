#pragma once
#include <JSL/Concepts.h>
#include <fstream>
#include <filesystem>
namespace JSL::IO
{
	
    namespace internal
    {
        /*!
         * Helper to verify file stream state. Throws an error if the file does not exist
         */
        void checkFile(const std::ofstream& file, const std::filesystem::path& path);

        /*! Throws an error indicating mismatched vector sizes; exposed in header to allow template functions call JSL::internal::FatalError, without needing to include it in this header!
        */
        void mismatchError();
    }

	/*! @brief A POD for holding the options passed to a writing function
	*/
	struct WriteOptions
	{
		//! The characters used to separate elements at 'column level'
		std::string ColumnDelimiter = " ";
		//! The characters used to separate elements at 'row level'
		std::string RowDelimiter = "\n";
		//! The value passed to ``std::setprecision`` to control the precision of floating point string conversions. Does not affect values converted to strings via other methods.
		size_t Precision = 10;
	};


	/*!
	 * @brief Writes a container (or a series of container) to file at the specified path. Each element is separated by a rowdelimiter, and if multiple containers are present, each element is separated by a column delimiter
	 * @tparam ...Ts A list of iterable containers
	 * @param filename The file to be written to (overwrites previous contents)
	 * @param opts A configuration object (see below). This argument is optional and may be omitted.
	 * @param ...vecs A variadic set of input containers, which must all have the same size.
	 */
	template<typename...Ts>
        requires ((JSL::Concept::NonStringRange<Ts> && ...) && sizeof...(Ts) > 0)
    void inline writeRows(const std::filesystem::path& filename, 
                                    WriteOptions opts,
                                    const Ts&... vecs)
    {

        // Verify all vectors match this size using a fold expression
        const size_t rowCount = std::get<0>(std::forward_as_tuple(vecs...)).size();
        if constexpr (sizeof...(vecs) > 1)
        {
            const bool allEqual = ((vecs.size() == rowCount) && ...);
            if (!allEqual)
            {
                internal::mismatchError();
            }
        }

        auto its = std::make_tuple(vecs.begin()...);
        auto end = std::get<0>(std::make_tuple(vecs.end()...));
        std::ofstream file(filename, std::ios::out);
        internal::checkFile(file, filename);
        file << std::setprecision(opts.Precision);
        
        while (std::get<0>(its) != end)
        {
            bool isFirst = true;
            std::apply([&isFirst,&file,&opts](auto&...it){
                ((file << (isFirst ? (isFirst = false, "") : opts.ColumnDelimiter) << *it), ...);
            },its);
            std::apply([](auto&...it){(++it,...);},its);
            file << opts.RowDelimiter;
        }
        file.close();
    }
	/*! @brief An overload of writeVector which uses the default writeoptions. 
	 * @tparam ...Ts A list of iterable containers
	 * @param filename The file to be written to (overwrites previous contents)
	 * @param ...vecs A variadic set of input containers, which must all have the same size.
	*/
    template<typename...Ts>
        requires (JSL::Concept::NonStringRange<Ts> && ...)
    void inline writeRows(const std::filesystem::path& filename, 
                                    const Ts&... vecs)
	{
		writeRows(filename,WriteOptions{},vecs...);
	}
	/*!
	 * @brief Writes a container-of-containers to file at the specified path. Each of the inner elements is separated by a column delimiter, and each of the outer elements is separated by a row delimiter 
	 * @tparam Outer a container type (corresponding to the columns)
	 * @tparam Inner an iterable range or tuple (corresponding to the rows) 
	 * @param filename The file to be written to (overwrites previous contents)
	 * @param opts A configuration object (see below). This argument is optional and may be omitted.
	 * @param contentMatrix The tabular-like object to be written to file	 
	 */
    template<template<typename...> class Outer, JSL::Concept::NonStringRange Inner, typename... Args>
    void inline writeRows(  const std::filesystem::path& filename, 
                                    WriteOptions opts,
                                    const Outer<Inner,Args...> & contentMatrix)
    {
		//[[note: the tuple version is actually handled by a different overload below; the claim above is purely for the doxygen documentation!]]
        std::ofstream file(filename, std::ios::out);
        internal::checkFile(file, filename);
        file << std::setprecision(opts.Precision);
        for (const auto& row : contentMatrix)
        {
            bool first = true;
            for (auto & r : row)
            {
                if (!first) { file << opts.ColumnDelimiter;}
                else        {  first = false;}
                file << r;
            }
            file << opts.RowDelimiter;
        }
        file.close();
    }

	// The tuple overload for the above function
	template<template<typename...> class Outer, JSL::Concept::TupleLike Inner, typename...Args>
	void inline writeRows(const std::filesystem::path & filename,
							WriteOptions opts,
							const Outer<Inner,Args...> & setOfTuples)
	{
		
        std::ofstream file(filename, std::ios::out);
        internal::checkFile(file, filename);
        file << std::setprecision(opts.Precision);
        for (const auto& row : setOfTuples)
		{
		    bool first = true;
			std::apply([&first, &file, &opts](auto&&... element){
				((file << (first ? (first = false, "") : opts.ColumnDelimiter) << element), ...);
			}, row);
		    file << opts.RowDelimiter;
		}
        file.close();
	}

	/*!
	 * @brief An overload of writeRows which uses the default write options. 
	  * @tparam Outer a container type (corresponding to the columns)
	 * @tparam Inner an iterable range (corresponding to the rows)
	 * @param filename The file to be written to (overwrites previous contents)
	 * @param opts A configuration object (see below). This argument is optional and may be omitted.
	 * @param contentMatrix The tabular-like object to be written to file	 
	 */
    template<template<typename...> class Outer, JSL::Concept::NonStringRange Inner, typename... Args>
    void inline writeRows(  const std::filesystem::path& filename, 
                                    const Outer<Inner,Args...> & contentMatrix)
	{
		writeRows(filename,WriteOptions{},contentMatrix);
	}
}