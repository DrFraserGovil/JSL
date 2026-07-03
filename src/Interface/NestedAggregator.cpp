#include <JSL/Interface/NestedAggregator.h>

namespace JSL::Interface
{
#define INSERT(type) {typeid(type).name(), #type}
	inline const std::map<std::string, std::string> CommonTypes = {
		INSERT(int),
		INSERT(double),
		INSERT(std::string),
		INSERT(bool),
		INSERT(char),
		INSERT(std::vector<int>),
		INSERT(std::vector<std::string>),
		INSERT(std::vector<double>),
		INSERT(size_t)};
#undef INSERT
	std::string makeTypeString(const std::type_info &t)
	{
		std::string tname = t.name();

		auto it = CommonTypes.find(tname);
		return (it != CommonTypes.end()) ? CommonTypes.at(tname) : tname;
	}

} // namespace JSL::Interface
