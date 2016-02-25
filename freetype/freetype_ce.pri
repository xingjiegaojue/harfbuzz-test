INCLUDEPATH += $$PWD/include

DEFINES += ENABLE_FREETYPE
			
build_pass:CONFIG(debug,debug|release) {
	LIBS += $$PWD/lib/wince/freetype248.lib
}

build_pass:CONFIG(release,debug|release) {
	LIBS += $$PWD/lib/wince/freetype248.lib
}

include ($$PWD/libcconv/libcconv_ce.pri)
include ($$PWD/src/src.pri)