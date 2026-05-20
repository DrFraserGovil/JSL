#include <JSL/Strings.h>
#include <JSL/internal/error.h>
#include <JSL/Vectors/Join.h>
#include <JSL/Display/Terminal.h>
namespace JSL::String
{
    size_t trueSize(std::string_view str)
    {
        return trueSize(str,JSL::Terminal::Global().TabSize());
    }
    size_t trueSize(std::string_view str,size_t tabSize)
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
                if (c == 'm')
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

    std::vector<std::string> wrap(std::string_view str, size_t width)
    {
        std::vector<std::string_view> lines;

        if (str.find("\n")!=std::string_view::npos)
        {
            std::vector<std::string> out;
            auto manuallines= split_view(str,"\n");
            for (auto line : manuallines)
            {
                JSL::Vector::append(out,wrap(line,width));
            }
            return out;
        }

        size_t currentLineStart = 0;
        
        size_t chunkStart = 0;
        size_t currentSize = 0;
        
        const char notWS = 'a'; //guaranteed to not be a whitespace character!
        char prevWhitespace = notWS;

        for (size_t i = 0; i <= str.size(); ++i)
        {
            char c = ' '; //we pretend there's an extra space on the end...
            if (i < str.size())
            {
                c = str[i];
            }
            if (std::isspace(c) )
            {
                if (prevWhitespace != c) //not repeated whitespace, or new whitespace
                {
                    if (i > 0)
                    {
                        //terminate the old block
                        auto lineToWord = str.substr(currentLineStart, i - currentLineStart);
                        size_t wordSize = trueSize(lineToWord) - currentSize;
                        //have to do this way round as characters like tabs have varying sizes depending on what came before!
                        if (currentSize + wordSize > width)
                        {
                            if (chunkStart > currentLineStart)
                            {
                                lines.push_back(str.substr(currentLineStart, chunkStart - currentLineStart));
                                currentLineStart = chunkStart;
                                currentSize = wordSize;
                            }
                            else
                            {
                                lines.push_back(str.substr(currentLineStart,i-currentLineStart));
                                currentLineStart = i;
                                currentSize = 0;
                            }
                        }
                        else
                        {
                            currentSize += wordSize;
                        }
                    }
                    chunkStart = i;
                }
                prevWhitespace = c;
            }
            else
            {
                if(prevWhitespace != notWS)
                {
                    chunkStart = i;
                    prevWhitespace = notWS;
                }
            }

            
        }
        auto finalLine = str.substr(currentLineStart);
        if (trueSize(finalLine) > 0)
        {
            lines.push_back(finalLine);
        }
        std::vector<std::string> out;
        for (auto & line : lines)
        {
            size_t lineSize = trueSize(line);

            if (lineSize > width)
            {
                while (line.back() == ' ' || line.back() == '\t')
                {
                    line.remove_suffix(1);
                }
                lineSize = trueSize(line);
            }


            //have to repad in case we removed a tab which damaged things!
            if (lineSize < width)
            {
                out.push_back((std::string)line + std::string(width - lineSize, ' '));
            }
            else
            {
                out.push_back((std::string)line);
            }
        }
        return out;
    }

    std::string wrapToString(std::string_view str, size_t width,std::string_view delim)
    {
        return stitch(wrap(str,width),delim);
    }


    std::string tableFormat(const std::vector<std::string_view> & input, size_t width, std::string_view delimiter,std::string_view endCap)
    {
        return tableFormat(input, std::vector<size_t>(input.size(),width),delimiter);
    }

    std::string tableFormat(const std::vector<std::string_view> & input, std::vector<size_t> widths, std::string_view delimiter, std::string_view endCap)
    {
        if (input.size() != widths.size())
        {
            JSL::internal::FatalError("Size mismatch",JSL_LOCATION) << "Input size (" << input.size() << ") must match widths (" << widths.size() <<") for column splitting";
        }
        size_t dsize = trueSize(delimiter);
        size_t esize = trueSize(endCap);

        //deduct the delimiter size from (n-1) cols
        size_t Ne = widths.size() - 1;
        for (size_t i = 0; i < Ne; ++i)
        {
            widths[i] = std::max(1ul,widths[i]-dsize);
        }
        //deduct the endcap size from the last cal
        widths[Ne]= std::max(1ul,widths[Ne]-esize);

        std::vector<std::vector<std::string>> linesplitInputs;
        size_t maxL = 0;
        for (size_t i = 0; i < input.size(); ++i)
        {
            std::vector<std::string> foldedColumn;
            auto lines = split_view(input[i], "\n");
            for (auto split : lines)
            {
                auto folded = wrap(split,widths[i]);
                JSL::Vector::append(foldedColumn, folded);
            }
            linesplitInputs.push_back(foldedColumn);
            maxL = std::max(maxL,foldedColumn.size());
        }
        std::ostringstream os;
        for (size_t i = 0; i < maxL; ++i)
        {
            for (size_t c = 0; c < input.size(); ++c)
            {
                if (c > 0){os << delimiter;}
                if (i < linesplitInputs[c].size())
                {
                    os << linesplitInputs[c][i];
                }
                else
                {
                    os << std::string(widths[c],' ');
                }
            }
            os <<endCap << "\n";
        }
        return os.str();
    }

}