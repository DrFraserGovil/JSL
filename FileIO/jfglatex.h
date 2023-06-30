#pragma once
#include "locationExists.h"
#include "mkdir.h"
#include <vector>
#include <string>
void JFG_Latex(std::string filename, std::string compilename, bool openfile, bool quietmode)//a C++ implementation of my jfglatex bash script, to prevent carting it around
{
	std::string texTail = ".tex";
	std::string::size_type i = filename.find(texTail);
	if (i != std::string::npos)
	{
		filename.erase(i,texTail.length());
	}
	std::string cmd = "pdflatex " + filename;
	if (quietmode)
	{
		cmd += " > /dev/null"; //pipes to null to prevent printing
	}
	cmd += "; ";
	// cmd += "biblatex" //do not need biblatex for this implementation
	cmd += cmd; //double up compilation to get references working
	if (compilename.find(".pdf") == std::string::npos)
	{
		compilename += ".pdf";
	}
	
	cmd += "mv " + filename + ".pdf " + compilename + ";";
	if (JSL::locationExists("_build"))
	{
		cmd = "mv _build/* ./; " + cmd;
		// system(mvcmd.c_str());
	}
	else
	{
		JSL::mkdir("_build");
	}

	if (openfile)
	{
		cmd += " open " + compilename + "; ";
	}

	std::vector<std::string> buildArray={"*.aux", "*.log", "*.toc", "*.mtc*", "*.blg", "*.bbl", "*.out", "*.maf", "*.fdb_latexmk", "*.fls", "*-blx.bib", "*xml", ".snm", "*.nav"};

	for (int i = 0; i < buildArray.size(); ++i)
	{
		cmd += "[ -f " + buildArray[i] + " ] && mv " + buildArray[i] + " _build/;";
		// system(cmd.c_str());
	}
	system(cmd.c_str());
}