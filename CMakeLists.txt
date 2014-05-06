cmake_minimum_required(VERSION 2.8.12.2)
project(generic_gui)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -g3")

function(MY_QT5_WRAP_UI outfiles )
        set(options)
        set(oneValueArgs)
        set(multiValueArgs OPTIONS)

        cmake_parse_arguments(_WRAP_UI "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

        set(ui_files ${_WRAP_UI_UNPARSED_ARGUMENTS})
        set(ui_options ${_WRAP_UI_OPTIONS})

        foreach(it ${ui_files})
                get_filename_component(outfile ${it} NAME_WE)
                get_filename_component(infile ${it} ABSOLUTE)
                set(outfile ${CMAKE_CURRENT_SOURCE_DIR}/ui_${outfile}.hpp)
                add_custom_command(OUTPUT ${outfile}
                        COMMAND ${Qt5Widgets_UIC_EXECUTABLE}
                        ARGS ${ui_options} -o ${outfile} ${infile}
                        MAIN_DEPENDENCY ${infile}
                        VERBATIM)
                list(APPEND ${outfiles} ${outfile})
        endforeach()
        set(${outfiles} ${${outfiles}} PARENT_SCOPE)
endfunction()

find_package(Qt5Widgets REQUIRED)
add_definitions(${Qt5Widgets_DEFINITIONS})
include_directories(SYSTEM ${Qt5Widgets_INCLUDE_DIRS})

find_package(Qt5Gui REQUIRED)
add_definitions(${Qt5Gui_DEFINITIONS})
include_directories(SYSTEM ${Qt5Gui_INCLUDE_DIRS})

find_package(Qt5Core REQUIRED)
add_definitions(${Qt5Core_DEFINITIONS})
include_directories(SYSTEM ${Qt5Core_INCLUDE_DIRS})

qt5_wrap_cpp(qt_sources
    gui_item_delegate.hpp
)

add_executable(generic_gui
	fusion_model.hpp
	index_list.hpp
	qt_adapter.hpp
	friendly_fusion.hpp
	fusion_static_dispatch.hpp
	meta_types.hpp
	boost_any_qvariant_convert.hpp
	fusion_outputter.hpp
	gui_item_delegate.hpp gui_item_delegate.cpp
	main_window.hpp main_window.cpp
	${qt_sources}
	main.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} ${Qt5Gui_EXECUTABLE_COMPILE_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")
target_link_libraries(generic_gui ${Qt5Widgets_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Core_LIBRARIES})



