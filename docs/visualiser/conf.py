TARGET = ["/home/jfg/CodeProjects/JSL/include/", "/home/jfg/CodeProjects/JSL/src/"]
SHOW_SOURCE = False
INCLUDE_SOURCE = True
DELETE_DOT = False

OUTFILE = ["heirarchy.png", "heatmap.png", "heirarchy.pdf", "heatmap.pdf"]
ENGINE = ["dot", "sfdp", "dot", "sfdp"]

IGNORE_GLOBS = "**/internal/*"
STRIP_NAMES = ["include", "src", "JSL"]

LIBRARY_ENTRY = "JSL.h"
LIBRARY_NAME = "JSL Library"
COLOR_SCHEME = ["group", "link", "group", "link"]
HORIZONTAL = True
NO_CROSSLINK = ["*Concepts.h"]
MERGE_LINKS = True
CROSSLINK_STRENGTH = 0.3