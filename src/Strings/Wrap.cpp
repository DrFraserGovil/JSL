#include <JSL/Display/Terminal.h>
#include <JSL/Strings/Split.h>
#include <JSL/Strings/Stitch.h>
#include <JSL/Strings/Wrap.h>
#include <JSL/Vectors/Join.h>
#include <JSL/internal/error.h>
namespace JSL::String
{
	bool isANSITerminator(char c)
	{
		return (c >= '@' && c <= '~' && c != '[');
	}
	size_t trueSize(std::string_view str)
	{
		return trueSize(str, JSL::Display::Terminal().TabSize());
	}
	size_t trueSize(std::string_view str, size_t tabSize)
	{
		size_t size = 0;
		bool inEscape = false;
		for (char c : str)
		{
			if (c == '\x1b')
			{
				inEscape = true;
				continue;
			}

			if (inEscape)
			{
				// this is the range of allowed ANSI termination sequences
				if (isANSITerminator(c))
				{
					inEscape = false;
				}
			}
			else
			{
				if (c == '\t')
				{
					size_t overflow = size % tabSize;
					size += tabSize - overflow; // Move to the next tab stop
				}
				else
				{
					++size;
				}
			}
		}
		return size;
	}

	size_t carefullySplitWord(std::string &line, size_t width, std::vector<std::string> &lineList)
	{
		// word, so we no know whitespace: only have to be careful about escape sequences
		size_t prev = 0;
		size_t size = 0;
		bool inEscape = false;
		std::string_view sv(line);
		std::string_view snippet;
		for (size_t idx = 0; idx < line.size(); ++idx)
		{
			char c = sv[idx];

			if (inEscape)
			{
				inEscape = !isANSITerminator(c);
			}
			else
			{
				if (c == '\x1b')
				{
					inEscape = true;
				}
				else
				{

					size += 1; // only increment if not within an escape sequence
				}
			}
			if (size == width)
			{
				lineList.emplace_back(sv.substr(prev, idx + 1 - prev)); //+1 because we want to include the character we just scanned
				size = 0;
				prev = idx + 1; //+1 to place the beginning of the next word after the character we just scanned
			}
		}
		if (prev != line.size())
		{
			line = sv.substr(prev, line.size() - prev);
			return size;
		}
		else
		{

			line = "";
			return 0;
		}
	}

	void terminateWord(std::vector<std::string> &lineList, std::string &currentLine, std::string &currentWord, size_t &currentLineSize, size_t tabSize, size_t width)
	{
		if (!currentWord.empty())
		{
			auto wordSize = trueSize(currentWord, tabSize); // by definition, has no tabs, so just the (byte-size) - (byte-size of ANSI)
			if (currentLineSize + wordSize <= width)
			{
				// the new word fits on the row, so add it in to the accumulator
				currentLine += currentWord;
				currentLineSize += wordSize;
			}
			else
			{
				// the new word doesn't fit, so pad the old line with spaces

				if (currentLineSize == 0)
				{
					// check if size = 0 but line is not empty (i.e. hidden ansi)
					currentWord = currentLine + currentWord;
				}
				else
				{
					if (currentLineSize < width)
					{
						currentLine += std::string(width - currentLineSize, ' ');
					}

					lineList.push_back(currentLine);
				}
				// then start a new line with this next word

				if (wordSize > width)
				{
					wordSize = carefullySplitWord(currentWord, width, lineList);
				}
				currentLine = currentWord;
				currentLineSize = wordSize;
			}

			// then check if the new line (Either from the joining or the residual) needs immediately appending
			if (currentLineSize == width)
			{
				lineList.push_back(currentLine);
				currentLineSize = 0;
				currentLine = "";
			}
			currentWord = "";
		}
	}

	std::vector<std::string> wrap(std::string_view str, size_t width)
	{
		std::vector<std::string_view> lines;

		// handle manual line breaks recursively
		if (str.find("\n") != std::string_view::npos)
		{
			std::vector<std::string> out;
			auto manuallines = split_view(str, "\n");
			for (auto line : manuallines)
			{
				JSL::Vector::append(out, wrap(line, width));
			}
			return out;
		}

		// main function loop
		std::vector<std::string> output;
		std::string currentLine;
		std::string currentWord;
		auto tabSize = JSL::Display::Terminal().TabSize();
		currentLine.reserve(width);

		size_t currentLineSize = 0;
		bool inEscape = false;
		for (size_t idx = 0; idx < str.size(); ++idx)
		{
			auto c = str[idx];
			if (c == ' ' || c == '\t')
			{
				terminateWord(output, currentLine, currentWord, currentLineSize, tabSize, width);

				// now handle the new whitespace

				// only add whitespace if there's already a character on the line, or this is the first line to be broken
				if (!currentLine.empty() || output.empty())
				{
					// we know by definition we can fit at least one space in (otherwise we'd have previously hit the immediate append branch above)
					if (c == ' ')
					{
						currentLine.push_back(c);
						currentLineSize += 1;
					}
					else
					{
						size_t tabStep = tabSize - currentLineSize % tabSize;
						tabStep = std::min(tabStep, width - currentLineSize); // tab should just take to end of line
						currentLine += std::string(tabStep, ' ');
						currentLineSize += tabStep;
					}

					if (currentLineSize >= width)
					{
						output.push_back(currentLine);
						currentLine = "";
						currentLineSize = 0;
					}
				}
			}
			else
			{
				currentWord.push_back(c);
			}
		}

		// flush the remaining words and lines
		terminateWord(output, currentLine, currentWord, currentLineSize, tabSize, width);
		if (!currentLine.empty())
		{
			if (currentLineSize == 0) // trailing ANSI sequence
			{
				if (output.empty()) // non contents, just ASCI
				{
					output.push_back(currentLine); // non padded line with ASCII colouration
				}
				else
				{
					output.back() += currentLine;
				}
			}
			else
			{
				if (currentLineSize < width)
				{
					currentLine += std::string(width - currentLineSize, ' ');
				}
				output.push_back(currentLine);
			}
		}
		return output;
	}

	std::string wrapToString(std::string_view str, size_t width, std::string_view delim)
	{
		return stitch(wrap(str, width), delim);
	}

	std::string tableFormat(const std::vector<std::string_view> &input, size_t width, std::string_view delimiter, std::string_view endCap)
	{
		return tableFormat(input, std::vector<size_t>(input.size(), width), delimiter);
	}

	std::string tableFormat(const std::vector<std::string_view> &input, std::vector<size_t> widths, std::string_view delimiter, std::string_view endCap)
	{
		if (input.size() != widths.size())
		{
			JSL::internal::LibraryError("Size mismatch", JSL_LOCATION) << "Input size (" << input.size() << ") must match widths (" << widths.size() << ") for column splitting";
		}
		size_t dsize = trueSize(delimiter);
		size_t esize = trueSize(endCap);

		// deduct the delimiter size from (n-1) cols
		size_t Ne = widths.size() - 1;
		for (size_t i = 0; i < Ne; ++i)
		{
			widths[i] = std::max((size_t)1, widths[i] - dsize);
		}
		// deduct the endcap size from the last cal
		widths[Ne] = std::max((size_t)1, widths[Ne] - esize);

		std::vector<std::vector<std::string>> linesplitInputs;
		size_t maxL = 0;
		for (size_t i = 0; i < input.size(); ++i)
		{
			std::vector<std::string> foldedColumn;
			auto lines = split_view(input[i], "\n");
			for (auto split : lines)
			{
				auto folded = wrap(split, widths[i]);
				JSL::Vector::append(foldedColumn, folded);
			}
			linesplitInputs.push_back(foldedColumn);
			maxL = std::max(maxL, foldedColumn.size());
		}
		std::ostringstream os;
		for (size_t i = 0; i < maxL; ++i)
		{
			for (size_t c = 0; c < input.size(); ++c)
			{
				if (c > 0) { os << delimiter; }
				if (i < linesplitInputs[c].size())
				{
					os << linesplitInputs[c][i];
				}
				else
				{
					os << std::string(widths[c], ' ');
				}
			}
			os << endCap << "\n";
		}
		return os.str();
	}

} // namespace JSL::String
