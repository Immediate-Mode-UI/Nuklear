set(LIBRARY_NAME "nuklear")

include(GNUInstallDirs)

set(pub "nuklear.h")
source_group("Public Header Files" FILES "${pub}")

set(priv1
        "nuklear_internal.h" "nuklear_math.c" "nuklear_util.c"
        "nuklear_color.c" "nuklear_utf8.c" "nuklear_buffer.c"
        "nuklear_string.c" "nuklear_draw.c" "nuklear_vertex.c")
source_group("Private [1] Files" FILES "${priv1}")

set(extern "stb_rect_pack.h" "stb_truetype.h")
source_group("External Files" FILES "${extern}")

set(priv2
        "nuklear_font.c" "nuklear_input.c" "nuklear_style.c"
        "nuklear_context.c" "nuklear_pool.c" "nuklear_page_element.c"
        "nuklear_table.c" "nuklear_panel.c" "nuklear_window.c"
        "nuklear_popup.c" "nuklear_contextual.c" "nuklear_menu.c"
        "nuklear_layout.c" "nuklear_tree.c" "nuklear_group.c"
        "nuklear_list_view.c" "nuklear_widget.c" "nuklear_text.c"
        "nuklear_image.c" "nuklear_9slice.c" "nuklear_button.c"
        "nuklear_toggle.c" "nuklear_selectable.c" "nuklear_slider.c"
        "nuklear_progress.c" "nuklear_scrollbar.c" "nuklear_text_editor.c"
        "nuklear_edit.c" "nuklear_property.c" "nuklear_chart.c"
        "nuklear_color_picker.c" "nuklear_combo.c" "nuklear_tooltip.c")
source_group("Private [2] Files" FILES "${priv2}")


if (NOT BUILD_HEADER_ONLY)
    add_library("${LIBRARY_NAME}" "${pub}" "${priv1}" "${extern}" "${priv2}")

    target_link_libraries("${LIBRARY_NAME}" PRIVATE "${PROJECT_NAME}_compiler_flags")

    #include(GenerateExportHeader)
    #set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_export.h")
    #generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

    target_include_directories(
            "${LIBRARY_NAME}"
            PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
            #"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

    # setup the version numbering
    set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "${${PROJECT_NAME}_VERSION}")
    set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "${${PROJECT_NAME}_VERSION_MAJOR}")
endif (NOT BUILD_HEADER_ONLY)

#============================#
# Amalgamation (header-only) #
#============================#

function (omit_includes new_file_contents file_contents blacklist)
    set(_new_file_contents "${file_contents}")
    foreach (omit ${blacklist})
        string(REPLACE "#include \"${omit}\"" "" _new_file_contents "${_new_file_contents}")
        string(REPLACE "#include <${omit}>" "" _new_file_contents "${_new_file_contents}")
    endforeach (omit ${blacklist})
    set(${new_file_contents} "${_new_file_contents}" PARENT_SCOPE)
endfunction (omit_includes new_file_contents file_contents blacklist)

function (fix_comments new_contents contents)
    # TODO: Fix this `fix_comments` implementation
    string(REGEX REPLACE "^//\(.*\)" "/*\\1 */" _contents "${contents}")
    set(${new_contents} "${_contents}" PARENT_SCOPE)
endfunction (fix_comments new_contents contents)

if (BUILD_HEADER_ONLY)
    set(macro "NK")

    set(intro "HEADER")

    set(outro "LICENSE" "CHANGELOG" "CREDITS")

    set(intro_contents "")
    foreach (filename ${intro})
        file(READ "${filename}" file_contents)
        set(intro_contents "${file_contents}")
    endforeach (filename ${intro})

    set(pub_contents "")
    foreach (filename ${pub})
        file(READ "${filename}" file_contents)
        set(pub_contents "${file_contents}")
    endforeach (filename ${pub})

    set(priv_blacklist ${pub} ${priv1} ${priv2} ${extern})

    set(amalgam_contents "")
    foreach (filename ${priv1})
        file(READ "${filename}" file_contents)
        omit_includes(new_file_contents
                "${file_contents}"
                "${priv_blacklist}")
        string(APPEND amalgam_contents "${new_file_contents}")
    endforeach (filename ${priv1})

    foreach (filename ${extern})
        file(READ "${filename}" file_contents)
        fix_comments(new_file_contents "${file_contents}")
        string(APPEND amalgam_contents "${new_file_contents}")
    endforeach (filename ${extern})

    foreach (filename ${priv2})
        file(READ "${filename}" file_contents)
        omit_includes(new_file_contents
                "${file_contents}"
                "${priv_blacklist}")
        string(APPEND amalgam_contents "${new_file_contents}")
    endforeach (filename ${priv1})

    set(outro_contents "")
    foreach (filename ${outro})
        file(READ "${filename}" file_contents)
        string(APPEND outro_contents "${file_contents}")
    endforeach (filename ${outro})

    set(macro "NK")

    set(Header_Files "${CMAKE_SOURCE_DIR}/nuklear.h")
    file(WRITE "${Header_Files}"
            "/*\n"
            "${intro_contents}"
            "*/\n"
            "#ifndef ${macro}_SINGLE_HEADER\n"
            "  #define ${macro}_SINGLE_HEADER\n"
            "#endif\n\n"
            "${pub_contents}\n"
            "#endif /* ${macro}_SINGLE_HEADER */\n\n"

           "#ifdef ${macro}_IMPLEMENTATION\n"
           "${amalgam_contents}"
           "#endif /* ${macro}_IMPLEMENTATION */\n"

           "/*\n"
           "${outro_contents}"
           "*/\n\n")
    source_group("Header Files" FILES "${Header_Files}")

    add_library("${LIBRARY_NAME}" INTERFACE "${Header_Files}")

    target_link_libraries("${LIBRARY_NAME}" INTERFACE "${PROJECT_NAME}_compiler_flags")
endif (BUILD_HEADER_ONLY)

#====================#
# Package to install #
#====================#

install(FILES ${Header_Files}
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_NAME}")

set(installable_libs "${LIBRARY_NAME}" "${PROJECT_NAME}_compiler_flags")
if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
install(TARGETS ${installable_libs}
        EXPORT "${LIBRARY_NAME}Targets"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(EXPORT "${LIBRARY_NAME}Targets" DESTINATION "${CMAKE_INSTALL_DATADIR}/${LIBRARY_NAME}")
