#pragma once

/*
	The JSL (Jack Standard Library) is a collection of code written by Jack Fraser-Govil (https://orcid.org/0000-0002-6980-8484) during the course of his PhD (2018-2022)

	This code is intended to capture several elements of boilerplate code that I ended up writing again and again across several projects.

	It shouldn't be of any interest to anyone else, but just in case, it is distributed under a GNU v3 public license. Go nuts, weirdos.
*/
#define JSL_ACTIVE // define a preprocessor variable which allows other libraries to conditionally install off it

#if defined(_WIN32) || defined(_WIN64)
#include <JSL/Async.h>
#endif
#include <JSL/Concepts.h>
#include <JSL/Display.h>
#include <JSL/IO.h>
#include <JSL/Parameters.h>
#include <JSL/Strings.h>
#include <JSL/Time.h>
#include <JSL/Vectors.h>

// Log adds a macro into the global namespace, so be a bit careful about including it
#ifdef JSL_REMOVE_LOG
// undef it just in case it snuck into the other header-includes (it shouldn't have, but it pays to be careful)
#undef LOG
#else
#include <JSL/Log.h>
#endif
