#include <JSL/Display/ProgressBar.h>
#include <JSL/Display/Terminal.h>
#include <JSL/Log.h>
#include <JSL/internal/error.h>
#include <sstream>
namespace JSL::Display::Progress
{
	namespace internal
	{

		ProgressEngine::ProgressEngine(std::vector<size_t> &&lengths) : Length(std::move(lengths)), Depth(Length.size())
		{
			if (Depth == 0)
			{
				JSL::internal::FatalError("Bad progress bar", JSL_LOCATION) << "Depth cannot be 0!";
			}
			Progress = std::vector<size_t>(Depth, 0);
			MarkCount = std::vector<size_t>(Depth, 0);
			Suffixes.resize(Depth, "");
			Prefixes.resize(Depth, "");
		}

		void ProgressEngine::Tick()
		{
			if (Finished) return;
			int idx = Depth - 1;
			while (idx >= 0)
			{
				++Progress[idx];
				if (Progress[idx] < Length[idx])
				{
					CheckMarks(idx);
					return;
				}
				else
				{
					Progress[idx] = 0;
					--idx;
				}
			}

			Finished = true;
			Progress = Length;
			CheckMarks(0);
		}

		void ProgressEngine::Update(size_t position)
		{
			// have to be a bit careful with 0, as it can underflow!
			if (position > 0)
			{
				Progress[Depth - 1] = position - 1;
				Tick();
			}
			else
			{
				Progress[Depth - 1] = 0;
				CheckMarks(0);
			}
		}
		void ProgressEngine::SetPrefix(std::string prefix)
		{
			SetPrefix(prefix, Depth - 1);
		}
		void ProgressEngine::SetPrefix(std::string prefix, size_t idx)
		{
			Prefixes[idx] = prefix;
		}
		void ProgressEngine::SetPrefix(std::vector<std::string> &prefixes)
		{
			if (prefixes.size() != Depth)
			{
				JSL::internal::FatalError("Prefix size mismatch", JSL_LOCATION) << "Tried to assign " << prefixes.size() << " prefixes, but needed " << Depth;
			}
			Prefixes = prefixes;
		}

		void ProgressEngine::SetSuffix(std::string suffix)
		{
			SetSuffix(suffix, Depth - 1);
		}
		void ProgressEngine::SetSuffix(std::string suffix, size_t idx)
		{
			Suffixes[idx] = suffix;
		}

		void ProgressEngine::SetSuffix(std::vector<std::string> &suffixes)
		{

			if (suffixes.size() != Depth)
			{
				JSL::internal::FatalError("Prefix size mismatch", JSL_LOCATION) << "Tried to assign " << suffixes.size() << " prefixes, but needed " << Depth;
			}
			Suffixes = suffixes;
		}

		void ProgressEngine::CheckMarks(int idx)
		{
			bool needsRender = false;
			for (size_t i = idx; i < Depth; ++i)
			{
				size_t expected = std::min(RenderLength, (size_t)(0.5 + (Progress[i] * RenderLength) * 1.0 / Length[i])); // old fashioned rounding
				if (expected != MarkCount[i])
				{
					MarkCount[i] = expected;
					needsRender = true;
				}
			}
			if (needsRender)
			{
				Render();
			}
		}
		void ProgressEngine::ThrowSizeMismatch(size_t actual)
		{
			JSL::internal::FatalError("Vector size mismatch", JSL_LOCATION) << "Positions size (" << actual << ") does not match depth (" << Depth << ")";
		}
	} // namespace internal

	////Animated bar
	Bar::Bar(size_t length, size_t barlength) : ProgressEngine({length})
	{
		RenderLength = barlength;
		Render();
	}
	Bar::Bar(std::vector<size_t> &&lengths, size_t barlength) : ProgressEngine(std::move(lengths))
	{
		RenderLength = barlength;
		Render();
	}

	void Bar::Render()
	{
		std::ostringstream buffer;
		if (hasRendered)
		{
			buffer << JSL::Display::Move(Display::Direction::Up, Depth);
			buffer << JSL::Display::MoveToColumn(0); // move all the way left
		}
		hasRendered = true;
		for (size_t i = 0; i < Depth; ++i)
		{
			buffer << Prefixes[i] << "[" << std::string(MarkCount[i], Symbol) << std::string(RenderLength - MarkCount[i], ' ') << "] " << Suffixes[i] << "\n";
		}
		std::cout << buffer.str() << std::flush;
	}

	/// static bar
	Static::Static(size_t length, size_t barlength) : ProgressEngine({length})
	{
		RenderLength = barlength;
	}

	void Static::Render()
	{
		std::ostringstream buffer;
		if (LastRender == 0)
		{
			buffer << "[";
		}
		int newmarks = (MarkCount[0] - LastRender);
		buffer << std::string(newmarks, Symbol);
		if (Finished)
		{
			buffer << "]\n";
		}
		LastRender = MarkCount[0];
		std::cout << buffer.str() << std::flush;
	}
} // namespace JSL::Display::Progress
