#pragma once
#include <string_view>
#include <vector>
namespace JSL::Log
{
    struct LineComponent
		{
			bool IsWhitespace = false;
			int Start;
			int End;
			std::string_view * window;
			size_t RealSize;
			LineComponent(int idx)
			{
				Start = idx;
			}
			void Terminate(std::string_view line,int idx)
			{
				window = &line;
				End = idx;
				ComputeSize(line);
			}
			std::string_view Snippet()
			{
				return window->substr(Start,End-Start);
			}
			void ComputeSize(std::string_view line)
			{
				RealSize = 0;
				bool inEscape = false;
				for (int i = Start; i < End; ++i)
				{
					if (line[i] == '\x1b')
					{
						inEscape = true;
						continue;
					}

					if (inEscape)
					{
						if (line[i] == 'm')
						{
							inEscape = false;
						}
					}
					else
					{
						++RealSize;
						if (line[i]=='\t')
						{
							RealSize += 3;
						}
					}
				}
			}
			static std::vector<LineComponent> GetAll(std::string_view line)
			{
				std::vector<LineComponent> bits;
				char notWS = 'a';
				char prevWhitespace = notWS;
				LineComponent current(0);
				current.Start = 0;
				for (size_t i = 0; i < line.size(); ++i)
				{
					bool isWhitespace = (line[i] == ' ') || (line[i] == '\t');
					if (isWhitespace)
					{
						if (prevWhitespace != line[i]) //not repeated whitespace, or new whitespace
						{
							current.Terminate(line,i);
							bits.push_back(current);
							current = LineComponent(i);
							current.IsWhitespace = true;
						}
						prevWhitespace = line[i];
					}
					else
					{
						if (prevWhitespace != notWS)
						{
							current.Terminate(line,i);
							bits.push_back(current);
							current = LineComponent(i);
						}
						prevWhitespace = notWS;
					}
				}
				current.Terminate(line,line.size());
				bits.push_back(current);
				return bits;
			}
		};
}