#pragma once
#include <array>
#include <cstdio>

namespace JSL::IO::internal
{

	//! A magic number associated with the tar block size. DO NOT CHANGE!
    inline constexpr size_t BLOCK_SIZE = 512;

    //! The number of empty blocks used to signal the end of an archive.
    inline constexpr size_t ARCHIVE_END_BLOCK_COUNTS = 2;
    //! The character sequence that indicates a tar-archive has ended
    inline constexpr char END_OF_ARCHIVE[ARCHIVE_END_BLOCK_COUNTS*BLOCK_SIZE]{0};

    //! A completely empty block of memory, used to pad out incomplete blocks
    inline constexpr char zeroBuffer[512]{0};

    //! @brief A struct for holding the header metadata for the overall tar structure -- constrained to be exactly 512 bytes.
    //! Forgive the magic numbers: they are defined by the tar specification
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
    //! If this fails, the compiler is doing something funky, and you need to be worried.
    static_assert(sizeof(TarHeader) == 512, "TarHeader must be exactly 512 bytes. Something bad has happened.");
}
