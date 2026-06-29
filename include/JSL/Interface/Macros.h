#pragma once
#include <stdexcept>
// Unpacking glue to strip tuple parentheses
#define JSL_UNPACK(macro, tuple) macro tuple

// Iteration engine (Extended to support up to 8 variables)
#define JSL_VEACH_1(macro, x) JSL_UNPACK(macro, x)
#define JSL_VEACH_2(macro, x, ...) JSL_UNPACK(macro, x) JSL_VEACH_1(macro, __VA_ARGS__)
#define JSL_VEACH_3(macro, x, ...) JSL_UNPACK(macro, x) JSL_VEACH_2(macro, __VA_ARGS__)
#define JSL_VEACH_4(macro, x, ...) JSL_UNPACK(macro, x) JSL_VEACH_3(macro, __VA_ARGS__)
#define JSL_VEACH_5(macro, x, ...) JSL_UNPACK(macro, x) JSL_VEACH_4(macro, __VA_ARGS__)
#define JSL_VEACH_6(macro, x, ...) JSL_UNPACK(macro, x) JSL_VEACH_5(macro, __VA_ARGS__)
#define JSL_VEACH_7(macro, x, ...) JSL_UNPACK(macro, x) JSL_VEACH_6(macro, __VA_ARGS__)
#define JSL_VEACH_8(macro, x, ...) JSL_UNPACK(macro, x) JSL_VEACH_7(macro, __VA_ARGS__)

#define JSL_GLUE(x, y) x##y
#define JSL_CONCAT(x, y) JSL_GLUE(x, y)

// This catches the overflow state and drops the poison pill token
#define JSL_VEACH_OVERFLOW(...) ERROR_MAXIMUM_OCCUPANCY_IS_8_VALUES
#define JSL_COUNT_ARGS(...) JSL_GET_COUNT(__VA_ARGS__, OVERFLOW, OVERFLOW, OVERFLOW, 8, 7, 6, 5, 4, 3, 2, 1)
#define JSL_GET_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, COUNT, ...) COUNT

#define JSL_FOR_EACH(macro, ...) \
	JSL_CONCAT(JSL_VEACH_, JSL_COUNT_ARGS(__VA_ARGS__))(macro, __VA_ARGS__)

// Specific expansion passes
#define JSL_PASS_CONTEXT(type, name, def, trigger, ...)                                                                                                                              \
	if (__jsl_macro_context.contains(trigger) || __jsl_macro_aliases.contains(trigger)) { throw std::invalid_argument("Trigger '" + std::string(trigger) + "' is already in use"); } \
	__jsl_macro_context[trigger] = JSL::Interface::MapTypeToKey<type>::value;

#define JSL_PASS_ALIAS(type, name, def, trigger, ...)                                                                                                                    \
	for (const std::string &al : std::vector<std::string>{__VA_ARGS__})                                                                                                  \
	{                                                                                                                                                                    \
		if (__jsl_macro_context.contains(al)) { throw std::invalid_argument("Alias '" + std::string(al) + "' collides with an existing primary trigger"); }              \
		if (__jsl_macro_aliases.contains(al)) { throw std::invalid_argument("Alias '" + std::string(al) + "' is already in use by '" + __jsl_macro_aliases[al] + "'"); } \
		__jsl_macro_aliases[al] = trigger;                                                                                                                               \
	}

#define JSL_PASS_PARSE(type, name, def, trigger, ...) \
	type name = __jsl_macro_cli.Parse<type>(trigger, def);

/* @brief  Main entry point macro
	@warning Cannot support more than 8 arguments
*/
#define PARSE_CONFIGURE(argc, argv, commandDestination, ...)                                                              \
	std::map<std::string, JSL::Interface::KeyType> __jsl_macro_context;                                                   \
	std::map<std::string, std::string> __jsl_macro_aliases;                                                               \
	__VA_OPT__(JSL_FOR_EACH(JSL_PASS_CONTEXT, __VA_ARGS__))                                                               \
	__VA_OPT__(JSL_FOR_EACH(JSL_PASS_ALIAS, __VA_ARGS__))                                                                 \
	auto __jsl_macro_cli = JSL::Interface::ConfigurableCommandLine(argc, argv, __jsl_macro_context, __jsl_macro_aliases); \
	__VA_OPT__(JSL_FOR_EACH(JSL_PASS_PARSE, __VA_ARGS__))                                                                 \
	commandDestination = __jsl_macro_cli.Commands;
