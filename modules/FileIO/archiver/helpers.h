#pragma once
#include <array>
#include <cstdio>
#include <string>
#include <span>
#include <cstddef>
#include <cstdint>
namespace JSL::Archiver::internal
{
    //! A magic number associated with the tar block size. DO NOT CHANGE!
    inline constexpr size_t BLOCK_SIZE = 512;
    inline constexpr char END_OF_ARCHIVE[2*internal::BLOCK_SIZE]{0};
    inline constexpr char zeroBuffer[512]{0};
    //A struct for holding the header metadata for the overall tar structure -- constrained to be exactly 512 bytes.
    // Forgive the magic numbers: they are defined by the tar specification
    struct TarHeader
    {
        std::array<char, 100> name{};
        std::array<char, 8> mode{};
        std::array<char, 8> uid{};
        std::array<char, 8> gid{};
        std::array<char, 12> size{};
        std::array<char, 12> mtime{};
        std::array<char, 8> chksum{' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
        char typeflag{'0'};
        std::array<char, 100> linkname{};
        std::array<char, 6> magic{'u', 's', 't', 'a', 'r', '\0'};
        std::array<char, 2> version{'0', '0'};
        std::array<char, 32> uname{};
        std::array<char, 32> gname{};
        std::array<char, 8> devmajor{};
        std::array<char, 8> devminor{};
        std::array<char, 155> prefix{};
        std::array<char, 12> padding{};  
    };
    static_assert(sizeof(TarHeader) == 512, "TarHeader must be exactly 512 bytes");

    struct Security
    {
        uint32_t UID = 1000; // uid of first normal human user. A good default as tar doesn't *really* need a uid (it's discarded when tar runs) 
        uint32_t GID = 1000;
        std::string UName = "user";
        std::string GName = "user";
        uint32_t Permission = 0644; //standard linux -rw-r--r-- permission
    };


    struct WriteExporter
    {
        TarHeader headerBlock;
        std::string data;
        size_t padding;
    };

    /*
		A container for recording the location of files in an archive stream
	*/
	struct ReadMetaData
	{
		std::string filename; //!< The name of the file - for comparing to user requests
		size_t file_size; //!< The size of the file (in bytes)
		std::streampos data_offset; //!< Byte offset to the file's data in the archive
	};
};