project (husky_test)
find_package (KDE4 REQUIRED)
find_package (Qt4 REQUIRED)
include (KDE4Defaults)
include (${QT_USE_FILE})
include_directories (${KDE4_INCLUDES})
include_directories (${PROJECT_SOURCE_DIR}/../../)
add_definitions(-DTESTCASE)

set (husky_FORMS
    ../../ui/openprojectlayout.ui

    ../../ui/queryviewlayout.ui
    ../../ui/autocompletionlayout.ui
    ../../ui/newprojectlayout.ui
    ../../ui/searchresultslayout.ui
    ../../ui/fileviewlayout.ui
    ../../ui/bookmarkslayout.ui)
QT4_WRAP_UI(husky_FORMS_HEADERS
    ${husky_FORMS})

set (husky_SRCS 
    ../../openprojectdlg.cpp

    ../../newprojectdlg.cpp
    main.cpp 
    mainwin.cpp
    ../../bookmarksdlg.cpp
    ../../queryview.cpp
    ../../queryresultsmenu.cpp
    ../../queryviewdriver.cpp
    ../../queryviewdlg.cpp
    ../../searchresultsdlg.cpp
    ../../frontend.cpp
    ../../cscopefrontend.cpp
    ../../configfrontend.cpp
    ../../projectbase.cpp
    ../../fileview.cpp
    ../../filelist.cpp
    ../../kscopeconfig.cpp
    ../../kscopepixmaps.cpp
    ../../searchlist.cpp)
kde4_add_executable (husky_test ${husky_SRCS} ${husky_FORMS_HEADERS})
target_link_libraries (husky_test ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KFILE_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS})
install(TARGETS husky_test ${INSTALL_TARGETS_DEFAULT_ARGS})

# see techbase.kde.org/Development/CMake/Addons_for_KDE for more libraries
