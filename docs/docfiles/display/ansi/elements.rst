ANSI Elements
==================

 A command of ``foreground -> red`` followed by ``foreground->green`` will naturally leave the stream with green-coloured text. However, provided that they don't overwrite each other, ANSI sequences can be composited together: ``foreground->red`` and ``style->bold`` results in red-bold text. 

 The JSL::Format::Element enum therefore acts to track which commands can be composited, and which overwrite each other. 



.. jsl:: JSL::Format::Element
	:file: Display/Format.h
	:command: doxygenenum
