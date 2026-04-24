#include <JSL/FileIO/fileWriters.h>

namespace JSL
{
    namespace internal
    {
        void checkFile(const std::ofstream& file, const std::filesystem::path& path)
        {
            if (!file.is_open())
            {
                FatalError("I/O Error") << "Could not open file: " << path.string();
            }
        }
    }

    void initialiseFile(const std::filesystem::path& filename)
    {
		std::ofstream file(filename, std::ios::out);
        internal::checkFile(file, filename);
        file.close();
	}


    void writeStringToFile(const std::filesystem::path& filename, std::string_view content,std::ios_base::openmode mode)
    {
        std::ofstream file(filename, mode);
        internal::checkFile(file, filename);
        file << content;
        file.close();
    }
}