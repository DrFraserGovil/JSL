#include <JSL/Strings/FoldLine.h>
#include <JSL/Strings/Manipulate.h>

namespace JSL
{
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

    std::vector<std::string> foldToWidth(std::string_view str, size_t width)
    {
        std::vector<std::string_view> lines;

        size_t currentLineStart = 0;
        
        size_t chunkStart = 0;
        size_t currentSize = 0;
        
        const char notWS = 'a'; //guaranteed to not be a whitespace character!
        char prevWhitespace = notWS;

        for (size_t i = 0; i < str.size(); ++i)
        {
            char c = str[i];
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
                            lines.push_back(str.substr(currentLineStart, chunkStart - currentLineStart));
                            currentLineStart = chunkStart;
                            currentSize = wordSize;
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



}