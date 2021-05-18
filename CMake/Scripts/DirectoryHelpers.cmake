macro (subdir_list result curdir)
    file (GLOB children RELATIVE ${curdir} ${curdir}/*)
    set (dirlist "")

    foreach (child ${children})
        if (IS_DIRECTORY ${curdir}/${child})
            list (APPEND dirlist ${child})
        endif ()
    endforeach ()

    set (${result} ${dirlist})
endmacro()

#

function (add_subdirectory_if_valid subdir)
    if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/${subdir}/CMakeLists.txt)
        add_subdirectory (${subdir})
    endif ()
endfunction()

#

function (add_all_subdirs)
    subdir_list (subDirs ${CMAKE_CURRENT_LIST_DIR})

    foreach (subdir ${subDirs})
        add_subdirectory_if_valid (${subdir})
    endforeach()
endfunction()

#

function (add_all_flagged_subdirs)
    subdir_list (subDirs ${CMAKE_CURRENT_LIST_DIR})

    foreach (subdir ${subDirs})
        set (buildflag "Build${subdir}")

        if (${buildflag})
            add_subdirectory_if_valid (${subdir})
        endif ()
    endforeach ()
endfunction()

#

function (add_subdirectories)
    if (NOT DEFINED BuildAll)
        set (BuildAll TRUE)
    endif()

    if (BuildAll)
        add_all_subdirs()
    else()
        add_all_flagged_subdirs()
    endif()
endfunction()

#