cmake_minimum_required(VERSION 3.8)

set(SAFEC_PARSER_LEXER_GENPATH ${CMAKE_BINARY_DIR}/generated)
set(SAFEC_LEXER_PATH "${SAFEC_PARSER_LEXER_GENPATH}/SafecLexer.lex.c")
set(SAFEC_PARSER_PATH "${SAFEC_PARSER_LEXER_GENPATH}/SafecParser.yacc.cpp")

# TODO: try to generate C++
add_custom_target(lexer_generate
    COMMAND flex -o ${SAFEC_LEXER_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/parser_generator/c89.lex
    BYPRODUCTS ${SAFEC_LEXER_PATH}
    COMMENT "Generating C89 lexer..."
)

add_custom_target(parser_generate
    COMMAND bison -d -Wno-conflicts-sr -o ${SAFEC_PARSER_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/parser_generator/c89.ypp
    BYPRODUCTS ${SAFEC_PARSER_PATH}
    COMMENT "Generating C89 parser..."
)

add_subdirectory(logger)

add_library(parser
    ${SAFEC_LEXER_PATH}
    ${SAFEC_PARSER_PATH}

    Parser.cpp
    Semantics.cpp
    SemNode.cpp
    walkers/SemNodeWalker.cpp
    walkers/WalkerPrint.cpp
)

add_library(safec::parser ALIAS parser)

target_link_libraries(parser
    PUBLIC
        CONAN_PKG::boost
    PRIVATE
        safec::logger
)

target_include_directories(parser
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SAFEC_PARSER_LEXER_GENPATH}
)

add_dependencies(parser lexer_generate parser_generate)
