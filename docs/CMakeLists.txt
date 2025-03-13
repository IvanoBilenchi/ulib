# Files and directories

set(SPHINX_INPUT_DIRECTORY "${ULIB_DOCS_DIR}")

# Doxygen configuration

set(DOXYGEN_FULL_PATH_NAMES NO)
set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
set(DOXYGEN_TOC_INCLUDE_HEADINGS 0)
set(DOXYGEN_DISTRIBUTE_GROUP_DOC YES)
set(DOXYGEN_TYPEDEF_HIDES_STRUCT YES)
set(DOXYGEN_FORCE_LOCAL_INCLUDES YES)
set(DOXYGEN_MAX_INITIALIZER_LINES 0)
set(DOXYGEN_WARN_NO_PARAMDOC YES)
set(DOXYGEN_WARN_AS_ERROR YES)
set(DOXYGEN_EXCLUDE_SYMBOLS "P_*" "p_*")
set(DOXYGEN_ALPHABETICAL_INDEX NO)
set(DOXYGEN_IGNORE_PREFIX "U" "u")
set(DOXYGEN_GENERATE_XML YES)
set(DOXYGEN_MACRO_EXPANSION YES)
set(DOXYGEN_PREDEFINED
    "ULIB_DEPRECATED(msg)=/** @@deprecated msg */"
    "ULIB_INLINE="
)
set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")
set(DOXYGEN_XML_OUTPUT_DIRECTORY "${DOXYGEN_OUTPUT_DIRECTORY}/xml")
set(DOXYGEN_ALIASES
    [[destructor{1}="@note The returned object must be destroyed by calling <tt>#\1()</tt>."]]
    [[func{1}="<tt>\1</tt>"]]
    [[type{1}="<tt>\1</tt>"]]
    [[val{1}="<tt>\1</tt>"]]
    [[alias=@noop]]
)
set(DOXYGEN_VERBATIM_VARS DOXYGEN_ALIASES)

find_program(DOXYGEN_PYTHON
             NAMES python3 python
             DOC "Path to the Python interpreter used to run Doxygen input filters")

if(DOXYGEN_PYTHON)
    set(DOXYGEN_INPUT_FILTER "${DOXYGEN_PYTHON} ${ULIB_UTILS_DIR}/doxygen/alias.py")
endif()

# Documentation targets

find_package(Doxygen)

if(DOXYGEN_FOUND)
    find_package(Sphinx)
    set(DOCS_TARGET_NAME ulib-docs)

    if(SPHINX_FOUND)
        set(DOXYGEN_TARGET_NAME ulib-doxygen)
        sphinx_add_docs("${DOCS_TARGET_NAME}" "${SPHINX_INPUT_DIRECTORY}" ARGS -W
                        COMMENT "Generating documentation via Sphinx")
        add_dependencies("${DOCS_TARGET_NAME}" "${DOXYGEN_TARGET_NAME}")
    else()
        set(DOXYGEN_TARGET_NAME "${DOCS_TARGET_NAME}")
    endif()

    doxygen_add_docs("${DOXYGEN_TARGET_NAME}" ${ULIB_PUBLIC_HEADERS}
                     COMMENT "Generating documentation via Doxygen")
endif()
