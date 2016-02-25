INCLUDEPATH += $$PWD/include

HEADERS += $$PWD/include/codeconvert.h

build_pass:CONFIG(debug,debug|release) {
	LIBS += $$PWD/lib/windows/libcconv.lib
}

build_pass:CONFIG(release,debug|release) {
	LIBS += $$PWD/lib/windows/libcconv.lib
}