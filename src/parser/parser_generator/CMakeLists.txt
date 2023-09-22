cmake_minimum_required(VERSION 3.8)

set(SAFEC_PARSER_LEXER_GENPATH ${CMAKE_BINARY_DIR}/generated)
set(SAFEC_LEXER_PATH "${SAFEC_PARSER_LEXER_GENPATH}/SafecLexer.lex.c")
set(SAFEC_PARSER_PATH "${SAFEC_PARSER_LEXER_GENPATH}/SafecParser.yacc.cpp")

set(SAFEC_LEXER_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/c89.lex)
set(SAFEC_PARSER_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/c89.ypp)

include(${PROJECT_SOURCE_DIR}/cmake/ParserGenerate.cmake)

safec_parser_generate(
    TARGET safec
    LEX_INPUT ${SAFEC_LEXER_SOURCE_PATH}
    LEX_OUTPUT ${SAFEC_LEXER_PATH}
    PARSE_INPUT ${SAFEC_PARSER_SOURCE_PATH}
    PARSE_OUTPUT ${SAFEC_PARSER_PATH}
)

add_library(parser_generated
    ${SAFEC_LEXER_PATH}
    ${SAFEC_PARSER_PATH}
)

target_include_directories(parser_generated
    PUBLIC
        ${SAFEC_PARSER_LEXER_GENPATH}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/parser
)

target_link_libraries(parser_generated
    PUBLIC
        safec::logger
        safec::config
        CONAN_PKG::boost
)

add_library(safec::parser_generated ALIAS parser_generated)

add_dependencies(parser_generated
    safec_lexer_generate
    safec_parser_generate
)

add_custom_target(parser_generated_lex_bison_sources
    SOURCES
        ${SAFEC_LEXER_SOURCE_PATH}
        ${SAFEC_PARSER_SOURCE_PATH}
        ${SAFEC_LEXER_PATH}
        ${SAFEC_PARSER_PATH}
)
