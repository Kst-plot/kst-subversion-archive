include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/extdate ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${CMAKE_SOURCE_DIR}/src/widgets ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstapp_LIB_SRCS
# view objects...
    kst2dplot.cpp
    kstborderedviewobject.cpp
    kstmetaplot.cpp
    kstplotbase.cpp
    kstplotgroup.cpp
    kstplotlabel.cpp
    ksttoplevelview.cpp
    kstviewarrow.cpp
    kstviewbox.cpp
    kstviewellipse.cpp
    kstviewlabel.cpp
    kstviewlegend.cpp
    kstviewline.cpp
    kstviewobject.cpp
    kstviewobjectfactory.cpp
    kstviewpicture.cpp
    kstviewwidget.cpp
    kstviewwindow.cpp
# widgets...
    kst2dplotwidget.cpp
		kled.cpp
    kstdatanotifier.cpp
    kstdebugnotifier.cpp
    kstlogwidget.cpp
    kstviewlabelwidget.cpp
    kstviewlegendwidget.cpp
    ktimezonecombo.cpp
# dialogs...
    datasourcemetadatadialog.cpp
    kstbasicdialog.cpp
    kstchoosecolordialog.cpp
    kstcsddialog.cpp
    kstcurvedialog.cpp
    kstdatadialog.cpp
    kstdebugdialog.cpp
    ksteditviewobjectdialog.cpp
    ksteqdialog.cpp
    kstfilterdialog.cpp
    kstfitdialog.cpp
    kstgraphfiledialog.cpp
    ksthsdialog.cpp
    kstimagedialog.cpp
    kstmonochromedialog.cpp
    kstplugindialog.cpp
    kstpsddialog.cpp
    kstquickstartdialog.cpp
    kstsettingsdlg.cpp
    kstvectorsavedialog.cpp
    kstvvdialog.cpp
# handlers...
    kstgfx2dplotmousehandler.cpp
    kstgfxarrowmousehandler.cpp
    kstgfxellipsemousehandler.cpp
    kstgfxlegendmousehandler.cpp
    kstgfxlinemousehandler.cpp
    kstgfxmousehandler.cpp
    kstgfxmousehandlerutils.cpp
    kstgfxpicturemousehandler.cpp
    kstgfxrectanglemousehandler.cpp
    kstgfxtextmousehandler.cpp
    kstgfxtlvmousehandler.cpp
# other...
    kstdatacollection-gui.cpp
    dialoglauncher-gui.cpp
    labelrenderer.cpp
    kstlegenddefaults.cpp
    kstnumbersequence.cpp
    kstuinames.cpp
    kstcurvedifferentiate.cpp
#    kstiface_impl.cpp
    statuslabel.cpp
    plotmimesource.cpp
    kstplotdrag.cpp
    kstviewobjectimagedrag.cpp
    kstvectortable.cpp
    kstmatrixtable.cpp
    kstfittable.cpp
    kstdrag.cpp
    kstfieldselect.cpp
    ksteventmonitor.cpp
    kstalignment.cpp
    extensionmgr.cpp
    kstdatamanager.cpp
    kstpluginmanager.cpp
    kstdoc.cpp
    emailthread.cpp
    kstmatrixdialog.cpp
    ksteventmonitorentry.cpp
# xxx    kstviewmanager.cpp
# xxx    kstviewstringsdialog.cpp
    updatethread.cpp
    kstvectordialog.cpp
# xxx    kstprintoptionspage.cpp
# xxx    kstscalarlistview.cpp
# xxx    kststringlistview.cpp
# xxx    kstviewscalarsdialog.cpp
# xxx    kstviewvectorsdialog.cpp
# xxx    kstviewmatricesdialog.cpp
# xxx    kstviewfitsdialog.cpp
# xxx    kstchangenptsdialog.cpp
# xxx    kstchangefiledialog.cpp
# xxx    kstdatawizard.cpp
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
# xxx    datawizard.ui
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
    mainwindow.ui
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

target_link_libraries(kstapp kstbase kstmath kstwidgets ${KDE4_KDECORE_LIBS} QtXml QtGui)

set_target_properties(kstapp PROPERTIES VERSION ${KST_LIBKST_VERSION} SOVERSION 1)
install(TARGETS kstapp 
	LIBRARY DESTINATION lib
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
