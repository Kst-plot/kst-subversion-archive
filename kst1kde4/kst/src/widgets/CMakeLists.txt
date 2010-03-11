include_directories(${CMAKE_SOURCE_DIR} ./ ${CMAKE_SOURCE_DIR}/src/extdate ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstwidgets_LIB_SRCS
    kstbackbuffer.cpp
    comboboxselection_i.cpp
    draggablelistbox.cpp
    draggablelistview.cpp
    editmultiplewidget.cpp
    fftoptionswidget.cpp
    plotlistbox.cpp
    scalareditor.cpp
    scalarselector.cpp
    stringeditor.cpp
    stringselector.cpp
    vectorlistview.cpp
    kstcombobox.cpp)

kde4_add_ui_files(kstwidgets_LIB_SRCS
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

kde4_add_library(kstwidgets SHARED ${kstwidgets_LIB_SRCS})

target_link_libraries(kstwidgets kio kstbase kstmath ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${QT_LIBRARIES})

set_target_properties(kstwidgets PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS kstwidgets 
	LIBRARY DESTINATION lib
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

