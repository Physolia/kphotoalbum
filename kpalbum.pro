SOURCES	+= main.cpp \
	thumbnailview.cpp \
	thumbnail.cpp \
	imagemanager.cpp \
	imageloader.cpp \
	options.cpp \
	imageinfo.cpp \
	viewer.cpp \
	listselect.cpp \
    mainview.cpp \
    imageconfig.cpp \
    imageclient.cpp \
    util.cpp \
    imagepreview.cpp

HEADERS	+= thumbnailview.h \
	thumbnail.h \
	imagemanager.h \
	imageloader.h \
	options.h \
	imageinfo.h \
	viewer.h \
	listselect.h \
    mainview.h \
    imageconfig.h \
    imageclient.h \
    util.h \
    imagepreview.h

FORMS	= mainviewui.ui \
	optionsdialog.ui \
	imageconfigui.ui

unix {
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  UI_DIR = .ui
}
!unix {
  MOC_DIR = _moc
  OBJECTS_DIR = _obj
  UI_DIR = _ui
}

TEMPLATE	=app
INCLUDEPATH	+= .
LANGUAGE	= C++
LIBS += -ljpeg
