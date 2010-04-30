include_directories(${CMAKE_SOURCE_DIR} ./ ${CMAKE_SOURCE_DIR}/src/extdate ${CMAKE_SOURCE_DIR}/src/libkde ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstwidgets_LIB_SRCS
    kstbackbuffer.cpp
    colorpalettewidget.cpp
    comboboxselection_i.cpp
    curveappearancewidget.cpp
    curveplacementwidget.cpp
    datarangewidget.cpp
    draggablelistbox.cpp
    draggablelistview.cpp
    editmultiplewidget.cpp
    fftoptionswidget.cpp
    matrixselector.cpp
    plotlistbox.cpp
    scalareditor.cpp
    scalarselector.cpp
    stringeditor.cpp
    stringselector.cpp
    vectorlistview.cpp
    vectorselector.cpp
    kstcombobox.cpp)

qt4_wrap_ui(kstwidgets_UI_HDRS
    comboboxselection.ui
    stringeditor.ui
    scalareditor.ui
    scalarselector.ui
    matrixselector.ui
    vectorselector.ui
    stringselector.ui
    editmultiplewidget.ui
    curveplacementwidget.ui
    curveappearancewidget.ui
    fftoptionswidget.ui
    datarangewidget.ui
    colorpalettewidget.ui)

kde4_add_library(kstwidgets SHARED ${kstwidgets_LIB_SRCS} ${kstwidgets_UI_HDRS})

target_link_libraries(kstwidgets kstkde kstbase kstmath ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${QT_LIBRARIES})

set_target_properties(kstwidgets PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS kstwidgets 
	LIBRARY DESTINATION lib
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

