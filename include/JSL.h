#pragma once

/*
    The JSL (Jack Standard Library) is a collection of code written by Jack Fraser-Govil (https://orcid.org/0000-0002-6980-8484) during the course of his PhD (2018-2022)

    This code is intended to capture several elements of boilerplate code that I ended up writing again and again across several projects. 

    It shouldn't be of any interest to anyone else, but just in case, it is distributed under a GNU v3 public license. Go nuts, weirdos. 
*/


#define JSL_ACTIVE //define a preprocessor variable which allows other libraries to conditionally install off it


#include <JSL/Display.h>
#include <JSL/FileIO.h>
#include <JSL/Parameters.h>
#include <JSL/Strings.h>
#include <JSL/Time.h>
#include <JSL/Vectors.h>

// #include "Vectors/Search.h"
// #include "Display/Display.h"
// #include "Strings/Strings.h"
// #include "FileIO/FileIO.h"

//Log adds a macro into the global namespace
// We undef it here unless explicitly asked for; this prevents pollution but allows us to use logging in our library
#ifndef JSL_INCLUDE_LOG
    #undef LOG
#endif

