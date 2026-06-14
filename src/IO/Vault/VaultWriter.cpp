#include "JSL/IO/Filesystem.h"
#include <JSL/IO/Vault/VaultWriter.h>
#include <filesystem>
#include <stdexcept>
#include <string_view>

namespace JSL::IO
{
	VaultWriter::VaultWriter()
	{
		Initialised = false;
	}
	
	VaultWriter::VaultWriter(std::string_view path, Policy policy)
	{
		Strictness = policy;
		Open(path);	
	}

	void VaultWriter::Open(std::string_view path)
	{
		bool strictmode = (Strictness = Policy::Strict);
		if (Initialised)
		{
			if (strictmode)
			{
				throw std::logic_error("Cannot open a stream which is already open (strictmode engaged)");
			}
			else
			{
				Close();	
			}
		}
		Name = path;	
		TempName = Name + ".part";
		
		if (strictmode)
		{
			if (std::filesystem::exists(Name))
			{
				throw std::logic_error("Cannot open a stream to " + Name + "; file already exists (strictmode engaged)");	
			}
		}
	}
}
