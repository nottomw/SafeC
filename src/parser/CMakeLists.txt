cmake_minimum_required(VERSION 3.8)

set(SAFEC_PARSER_LEXER_GENPATH ${CMAKE_BINARY_DIR}/generated)
set(SAFEC_LEXER_PATH "${SAFEC_PARSER_LEXER_GENPATH}/SafecLexer.lex.c")
set(SAFEC_PARSER_PATH "${SAFEC_PARSER_LEXER_GENPATH}/SafecParser.yacc.c")

add_library(generated_lexer_parser
    ${SAFEC_LEXER_PATH}
    ${SAFEC_PARSER_PATH}
)

target_include_directories(generated_lexer_parser
    PUBLIC
        ${SAFEC_PARSER_LEXER_GENPATH}
)

add_custom_target(lexer_generate
    COMMAND flex -o ${SAFEC_LEXER_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/parser_generator/c89.lex
    BYPRODUCTS ${SAFEC_LEXER_PATH}
    COMMENT "Generating C89 lexer..."
)

add_custom_target(parser_generate
    COMMAND bison -d -o ${SAFEC_PARSER_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/parser_generator/c89.yacc
    BYPRODUCTS ${SAFEC_PARSER_PATH}
    COMMENT "Generating C89 parser..."
)

add_dependencies(generated_lexer_parser lexer_generate parser_generate)

add_library(parser
    Parser.cpp
)

add_library(safec::parser ALIAS parser)

target_link_libraries(parser
    PUBLIC
        CONAN_PKG::boost
    PRIVATE
        generated_lexer_parser
)

add_dependencies(parser generated_lexer_parser)
