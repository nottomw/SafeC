cmake_minimum_required(VERSION 3.8)

set(SAFEC_PARSER_LEXER_GENPATH ${CMAKE_BINARY_DIR}/generated)
set(SAFEC_LEXER_PATH "${SAFEC_PARSER_LEXER_GENPATH}/SafecLexer.lex.c")
set(SAFEC_PARSER_PATH "${SAFEC_PARSER_LEXER_GENPATH}/SafecParser.yacc.cpp")

include(${PROJECT_SOURCE_DIR}/cmake/ParserGenerate.cmake)

safec_parser_generate(
    TARGET safec
    LEX_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/c89.lex
    LEX_OUTPUT ${SAFEC_LEXER_PATH}
    PARSE_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/c89.ypp
    PARSE_OUTPUT ${SAFEC_PARSER_PATH}
)

add_library(parser_generated
    ${SAFEC_LEXER_PATH}
    ${SAFEC_PARSER_PATH}
)

target_include_directories(parser_generated
    PUBLIC
        ${SAFEC_PARSER_LEXER_GENPATH}
        ${PROJECT_SOURCE_DIR}/parser
)

add_library(safec::parser_generated ALIAS parser_generated)

add_dependencies(parser_generated
    safec_lexer_generate
    safec_parser_generate
)
