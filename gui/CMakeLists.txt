set(qapitrace_SRCS
   apitrace.cpp
   apicalldelegate.cpp
   apitracecall.cpp
   apitracefilter.cpp
   apitracemodel.cpp
   loaderthread.cpp
   mainwindow.cpp
   main.cpp
 )

qt4_automoc(${qapitrace_SRCS})

set(qapitrace_UIS
   ui/mainwindow.ui)

QT4_WRAP_UI(qapitrace_UIS_H ${qapitrace_UIS})

#add_app_icon(qapitrace_SRCS ../icons/hi*-qapitrace.png)
link_directories(${LINK_DIRECTORIES} ${QJSON_LIBRARY_DIRS})
include_directories(${QT_INCLUDES} ${QJSON_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_definitions(-DBUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}/..")

add_executable(qapitrace ${qapitrace_SRCS} ${qapitrace_UIS_H})

target_link_libraries(qapitrace trace ${QJSON_LIBRARIES} ${QT_LIBRARIES} )

########### install files ###############

#install(TARGETS qapitrace ${INSTALL_TARGETS_DEFAULT_ARGS} )
#install( FILES qapitrace.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )