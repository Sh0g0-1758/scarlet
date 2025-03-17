#pragma once

#include <ast/ast.hh>
#include <string>
#include <vector>

namespace scarlet {
namespace symbolTable {
enum class linkage { INTERNAL, EXTERNAL };
enum class symbolType { VARIABLE, FUNCTION };

// This struct is used to store information about a symbol
// - name: updated name of the symbol according to scope resolution
// - link: linkage of the symbol
// - type: type of the symbol
// - typeDef: type definition of the symbol
//            (used for variables and functions)
//            (a vector because functions can have multiple args)
struct symbolInfo {
  std::string name;
  linkage link;
  symbolType type;
  std::vector<ast::ElemType> typeDef;
  bool isDefined = false;
};
} // namespace symbolTable
} // namespace scarlet