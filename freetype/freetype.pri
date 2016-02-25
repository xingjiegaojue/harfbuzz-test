INCLUDEPATH += $$PWD/include

DEFINES += ENABLE_FREETYPE
	
build_pass:CONFIG(debug,debug|release) {
	LIBS += $$PWD/lib/windows/freetype248.lib
}

build_pass:CONFIG(release,debug|release) {
	LIBS += $$PWD/lib/windows/freetype248.lib
}