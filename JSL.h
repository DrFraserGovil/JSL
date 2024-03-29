#pragma once

/*
The JSL (Jack Standard Library) is a collection of code written by Jack Fraser (https://orcid.org/0000-0002-6980-8484) during the course of his PhD (2018-2022)

This code is intended to capture several elements of boilerplate code that I ended up writing again and again across several projects. 
It shouldn't be of any interest to anyone else, but just in case, it is distributed under a GNU v3 public license. Go nuts, weirdos. 
*/


#define JSL_LIBRARY_INSTALLED //define a preprocessor variable which allows other libraries to conditionally install off it

#include "FileIO/FileIO.h"
#include "Strings/Strings.h"
#include "CommandArgs/CommandArgs.h"
#include "Maths/Maths.h"
#include "Array/Array.h"
#include "gnuplot/gnuplot.h"
#include "System/System.h"
#include "Display/Display.h"
// #include "Testing/Testing.h"

