#pragma once
#include <unordered_map>
#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "helpers.h"
#include "../../utils/jsl_error.h"
namespace JSL::Archiver
{
    enum class Mode
    {
        //! Puts the Archive into read mode
        Read,
		//! Puts the Archive in write mode. If a file exists with the archive name, it is replaced by a blank archive.
		Write,
		//! Puts the Archive in write mode. If a file exists with the archive name, it is read as an archive, and the contents copied across.
		Append,
    };

    namespace internal
    {
        class VaultBase
        {
            protected:
                std::string Name;
                Mode FileMode;
                bool StrictMode = false;
                bool Initialised = false;

                VaultBase(Mode mode) : FileMode(mode){};
            public:
                void SetStrictMode(bool enabled) {StrictMode = enabled;};
        };
    }

    //forward declaration of the base class
    template <Mode M>
    class Vault;
};