#ifndef H_parse_tree_ParseTreeFwd
#define H_parse_tree_ParseTreeFwd

namespace pljit::parse_tree {

/// Forward-declarations of all non-abstract parse tree nodes
class Identifier;
class Literal;
class GenericToken;
class FunctionDefinition;
class ParameterDeclarations;
class VariableDeclarations;
class ConstantDeclarations;
class DeclaratorList;
class InitDeclaratorList;
class InitDeclarator;
class CompoundStatement;
class StatementList;
class Statement;
class AssignmentExpression;
class AdditiveExpression;
class MultiplicativeExpression;
class UnaryExpression;
class PrimaryExpression;
/// Forward-decl. of abstract classes might be helpful for implementing visitors.
class ParseTreeNode;
class FlexibleChildrenBase;
class DeclaratorListWrapper;

} // namespace pljit::parse_tree

#endif
