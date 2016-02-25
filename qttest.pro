CONFIG += qtestlib

include ($$PWD/harfbuzz/harfbuzz.pri)
include ($$PWD/freetype/freetype.pri)

HEADERS += test.h
SOURCES += test.cpp main.cpp