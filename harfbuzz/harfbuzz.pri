DEFINES += NO_OPENTYPE
INCLUDEPATH += $$PWD

HEADERS += $$PWD/harfbuzz.h
SOURCES += $$PWD/harfbuzz-buffer.c \
           $$PWD/harfbuzz-gdef.c \
           $$PWD/harfbuzz-gsub.c \
           $$PWD/harfbuzz-gpos.c \
           $$PWD/harfbuzz-impl.c \
           $$PWD/harfbuzz-open.c \
           $$PWD/harfbuzz-stream.c \
           $$PWD/harfbuzz-shaper-all.cpp \