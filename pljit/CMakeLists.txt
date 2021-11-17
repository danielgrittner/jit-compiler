set(PLJIT_SOURCES
        # Common files
        common/SourceCodeManager.cpp
        common/References.cpp
        # Lexer files
        lexer/Token.cpp
        lexer/Lexer.cpp
        # Parser files
        parse_tree/ParseTree.cpp
        parser/Parser.cpp
        parse_tree/ParseTreeDotVisitor.cpp
        # AST files
        ast/AST.cpp
        ast/ASTDotVisitor.cpp
        exec/ExecutionContext.cpp
        # AST Analysis files
        analysis/SymbolTable.cpp
        analysis/SemanticAnalysis.cpp
        # AST Optimizer files
        optim/DeadCodeElimination.cpp
        optim/ConstantPropagation.cpp
        # JIT files
        Pljit.cpp
        )

add_library(pljit STATIC ${PLJIT_SOURCES})
target_include_directories(pljit PUBLIC ${CMAKE_SOURCE_DIR})

add_clang_tidy_target(lint_pljit ${PLJIT_SOURCES})
add_dependencies(lint lint_pljit)
