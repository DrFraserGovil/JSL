// #include <JSL/IO/archiver/base.h>
#include <JSL/IO/archiver/readStream.h>
#include <unordered_map>
#include <algorithm>

#include <cstring>
#include <vector>
#include <functional>
#include <string_view>
namespace JSL::Archiver
{
	//template specialisation for the vault-writer
	template<>
	class Vault<Mode::Read> : public internal::VaultBase
	{
		private:
			std::ifstream FileStream;
			std::unordered_map<std::string,ReadStream> FileIndex;
			bool ReadBlock(char* buffer);
			
			void BuildIndex();
		public:
			Vault();

			//! Initialises the object and immediately calls Open() @param archivePath The name of the archive on disk @param strictMode The ::Mode assigned to the object
			Vault(const std::string & archivePath, bool strictMode = false);

			~Vault() noexcept;
			void Open(std::string_view path);

			void Close();

			std::vector<std::string> ListFiles();

			ReadStream & operator[](const std::string & file);

			void ForLineIn(const std::string & file,std::function<void(std::string_view)> perLineFunction);

			template<typename...ColumnTypes, typename TupleFunctor>
			void ForTabularLineIn(const std::string & file,std::string delimiter,TupleFunctor perTupleFunction)
			{
				(*this).operator[](file).ForTabularLineIn<ColumnTypes...>(delimiter,perTupleFunction);
			}
	};

}