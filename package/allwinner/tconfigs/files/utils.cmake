if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()

# Helper function to add preprocesor definition of __FILE_BASENAME__
# to pass the filename without directory path for debugging use.
#
# Example:
#   define_file_basename_for_sources(my_target)
#
# Will add -DFILE_BASENAME="filename" for each source file depended on
# by my_target, where filename is the name of the file.
function(define_file_basename_for_sources targetname)
    get_target_property(source_files "${targetname}" SOURCES)
    foreach(sourcefile ${source_files})
        # Get source file's current list of compile definitions.
        get_property(defs SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS)
        # Add the __FILE_BASENAME__=filename compile definition to the list.
        get_filename_component(basename "${sourcefile}" NAME)
        list(APPEND defs "__FILE_BASENAME__=\"${basename}\"")
        # Set the updated compile definitions on the source file.
        set_property(
            SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS ${defs})
    endforeach()
endfunction()

# Helper macro to add third party component
#   @param control_variable: determine whether to add the srcs and libs
#   @param srcs_variable: variable to save the src files
#   @param srcs_dir: directory that the src files are placed in
#   @param depend_libs_variable: variable to save the dependency libraries
#   @param depend_libs: dependency libraries, use ";" to seperate
macro(add_third_party_component
        control_variable srcs_variable srcs_dir depend_libs_variable depend_libs)
    message(STATUS "${BoldYellow}${control_variable} = ${${control_variable}}${ColourReset}")
    if (${control_variable})
        aux_source_directory("${srcs_dir}" ${srcs_variable})
        set(${depend_libs_variable} ${depend_libs})
        add_definitions(-D${control_variable})
    endif()
endmacro()
