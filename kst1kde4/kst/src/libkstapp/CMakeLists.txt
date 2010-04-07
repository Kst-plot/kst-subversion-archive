include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/extdate ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${CMAKE_SOURCE_DIR}/src/widgets ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstapp_LIB_SRCS
    kstdatacollection-gui.cpp
    dialoglauncher-gui.cpp
    datasourcemetadatadialog.cpp
    labelrenderer.cpp
    kstlegenddefaults.cpp
    kstnumbersequence.cpp
    kstuinames.cpp
    kstcurvedifferentiate.cpp
#    kstiface_impl.cpp
    statuslabel.cpp
    plotmimesource.cpp
    kstdatadialog.cpp
    kstplotdrag.cpp
    kstviewobjectimagedrag.cpp
    kstvectortable.cpp
    kstmatrixtable.cpp
    kstfittable.cpp
    kstlogwidget.cpp
    kstplotbase.cpp
    kstviewlabel.cpp
    kstplotlabel.cpp
    kst2dplot.cpp
    kst2dplotwidget.cpp
    kstviewlegend.cpp
    kstviewbox.cpp
    kstviewline.cpp
    kstviewellipse.cpp
    kstviewpicture.cpp
    kstviewarrow.cpp
    kstviewwidget.cpp
    kstviewobjectfactory.cpp
    kstviewobject.cpp
    kstplotgroup.cpp
    kstborderedviewobject.cpp
    ksttoplevelview.cpp
    kstgfxmousehandler.cpp
    kstgfxmousehandlerutils.cpp
    kstgfxrectanglemousehandler.cpp
    kstgfxlinemousehandler.cpp
    kstgfxtextmousehandler.cpp
    kstgfxellipsemousehandler.cpp
    kstgfxpicturemousehandler.cpp
    kstgfxarrowmousehandler.cpp
    kstgfx2dplotmousehandler.cpp
    kstgfxlegendmousehandler.cpp
    kstgfxtlvmousehandler.cpp
    kstmetaplot.cpp
    kstdrag.cpp
    kstfieldselect.cpp
    kstcsddialog.cpp
    ksteqdialog.cpp
    ksthsdialog.cpp
    kstpsddialog.cpp
    kstimagedialog.cpp
    kstvvdialog.cpp
    kstquickstartdialog.cpp
    kstviewlabelwidget.cpp
    kstviewlegendwidget.cpp
    kstvectorsavedialog.cpp
    kstchoosecolordialog.cpp
    ksteventmonitor.cpp
    kstdebugdialog.cpp
    kstmonochromedialog.cpp
    kstalignment.cpp
    kstcurvedialog.cpp
    kstbasicdialog.cpp
    kstviewwindow.cpp
    ktimezonecombo.cpp
    kstdebugnotifier.cpp
    kstdatanotifier.cpp
    kstplugindialog.cpp
    kstfitdialog.cpp
    kstfilterdialog.cpp
    extensionmgr.cpp
    ksteditviewobjectdialog.cpp
    kstdatamanager.cpp
    kstpluginmanager.cpp
    kstviewstringsdialog.cpp
    updatethread.cpp
    kstvectordialog.cpp
    kstgraphfiledialog.cpp
    kstsettingsdlg.cpp
    emailthread.cpp
    kstprintoptionspage.cpp
    kstscalarlistview.cpp
    kststringlistview.cpp
    kstviewscalarsdialog.cpp
    kstviewvectorsdialog.cpp
    kstviewmatricesdialog.cpp
    kstviewfitsdialog.cpp
    kstchangenptsdialog.cpp
    kstchangefiledialog.cpp
    kstmatrixdialog.cpp
    ksteventmonitorentry.cpp
    kstdatawizard.cpp
    kstdoc.cpp
    kstviewmanager.cpp
    kst.cpp)

#KDE3_ADD_DCOP_SKELS(kstapp_LIB_SRCS
#    kstiface.h
#    kstiface_impl.h)

kde4_add_ui_files(kstapp_LIB_SRCS
    basicdialogwidget.ui
    changefiledialog.ui
    changenptsdialog.ui
    choosecolordialog.ui
    csddialogwidget.ui
    curvedialogwidget.ui
    curvedifferentiate.ui
    datamanager.ui
    datasourcemetadatadialog.ui
    datawizard.ui
    debugdialog.ui
    editviewobjectdialog.ui
    eqdialogwidget.ui
    eventmonitorwidget.ui
    extensiondlg.ui
    fieldselect.ui
    graphfiledialog.ui
    histogramdialogwidget.ui
    imagedialogwidget.ui
    kstdatadialog.ui
    kstsettingsdlg.ui
    kstviewmanager.ui
    matrixdialogwidget.ui
    monochromedialog.ui
    plugindialogwidget.ui
    pluginmanager.ui
    psddialogwidget.ui
    quickstartdialog.ui
    vectordialogwidget.ui
    vectorsavedialog.ui
    vectorviewdialogwidget.ui
    view2dplotwidget.ui
    viewfitsdialog.ui
    viewlabelwidget.ui
    viewlegendwidget.ui
    viewmatricesdialog.ui
    viewscalarsdialog.ui
    viewstringsdialog.ui
    viewvectorsdialog.ui)

kde4_add_library(kstapp SHARED ${kstapp_LIB_SRCS})

target_link_libraries(kstapp kdeprint kstwidgets)

set_target_properties(kstapp PROPERTIES VERSION ${KST_LIBKST_VERSION} SOVERSION 1)
install(TARGETS kstapp 
	LIBRARY DESTINATION lib
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
