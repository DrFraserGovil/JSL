#include <catch2/catch_test_macros.hpp>
#include "../test_utils/catch_extended.h"
#include "../../modules/FileIO/forLineIn.h"

TEST_CASE("Reads files correctly","[file][read][utility]")
{
	MockFile F;
	int linesWritten = 100;
	for (int i = 0; i < linesWritten; ++i)
	{
		F << "test\n";
	}

	int linesRead = 0;
	JSL::forLineIn(F.Path,
		[&](const std::string line)
		{
			REQUIRE(line == "test");
			linesRead++;
		}
	);
	REQUIRE(linesRead == linesWritten);
}