cmake_minimum_required(VERSION 3.8)

function(safec_parser_generate)
    set(options)
    set(oneValueArgs TARGET LEX_INPUT LEX_OUTPUT PARSE_INPUT PARSE_OUTPUT)
    set(multiValueArgs)

    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT ARG_TARGET)
        message(FATAL_ERROR "missing TARGET argument")
    endif()

    if (NOT ARG_LEX_INPUT)
        message(FATAL_ERROR "missing LEX_INPUT argument")
    endif()

    if (NOT ARG_LEX_OUTPUT)
        message(FATAL_ERROR "missing LEX_OUTPUT argument")
    endif()

    if (NOT ARG_PARSE_INPUT)
        message(FATAL_ERROR "missing PARSE_INPUT argument")
    endif()

    if (NOT ARG_PARSE_OUTPUT)
        message(FATAL_ERROR "missing PARSE_OUTPUT argument")
    endif()

    add_custom_target(${ARG_TARGET}_lexer_generate
        COMMAND flex -o "${ARG_LEX_OUTPUT}" "${ARG_LEX_INPUT}"
        BYPRODUCTS "${ARG_LEX_OUTPUT}"
        COMMENT "Generating C89 lexer..."
    )

    add_custom_target(${ARG_TARGET}_parser_generate
        COMMAND bison -d -o "${ARG_PARSE_OUTPUT}" "${ARG_PARSE_INPUT}"
        BYPRODUCTS "${ARG_PARSE_OUTPUT}"
        COMMENT "Generating C89 parser..."
    )
endfunction()
