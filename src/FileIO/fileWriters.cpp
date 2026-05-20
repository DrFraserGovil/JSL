#include <JSL/FileIO/fileWriters.h>
#include <JSL/internal/error.h>
namespace JSL::IO
{
    namespace internal
    {
        void checkFile(const std::ofstream& file, const std::filesystem::path& path)
        {
            if (!file.is_open())
            {
                JSL::internal::FatalError("I/O Error",JSL_LOCATION) << "Could not open file: " << path.string();
            }
        }

        void mismatchError()
        {
            JSL::internal::FatalError("Vector length mismatch",JSL_LOCATION) << "All vectors must be same length when writing simultaneously to file";
        }
    }

    void initialise(const std::filesystem::path& filename)
    {
		std::ofstream file(filename, std::ios::out);
        internal::checkFile(file, filename);
        file.close();
	}


    void writeString(const std::filesystem::path& filename, std::string_view content,std::ios_base::openmode mode)
    {
        std::ofstream file(filename, mode);
        internal::checkFile(file, filename);
        file << content;
        file.close();
    }
}