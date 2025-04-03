#pragma once

#include <ast/ast.hh>
#include <string>
#include <tools/constant/constant.hh>
#include <vector>

namespace scarlet {
namespace symbolTable {
enum class linkage { NONE, INTERNAL, EXTERNAL };
enum class symbolType { VARIABLE, FUNCTION };
// True -> There is a definition
// Tentative -> There is a global declaration without extern,
//              which means it will be initalized with 0
// False -> There is no definition
enum class defType { TRUE, TENTATIVE, FALSE };
enum class derivedSymbolType { POINTER_TYPE, ARRAY_TYPE };
class derivedSymbol {
private:
  ast::ElemType baseType;
  derivedSymbolType derivedType;
  int array_size;
  std::shared_ptr<derivedSymbol> child;

public:
  void set_base_type(ast::ElemType baseType) { this->baseType = baseType; }
  void set_derived_type(derivedSymbolType derivedType) {
    this->derivedType = derivedType;
  }
  void set_array_size(int array_size) { this->array_size = array_size; }
  void set_child(std::shared_ptr<derivedSymbol> child) {
    this->child = std::move(child);
  }
  ast::ElemType get_base_type() { return baseType; }
  derivedSymbolType get_derived_type() { return derivedType; }
  int get_array_size() { return array_size; }
  std::shared_ptr<derivedSymbol> get_child() { return child; }
};

// This struct is used to store information about a symbol
// - name:      updated name of the symbol according to scope resolution
// - link:      linkage of the symbol
// - type:      type of the symbol
// - typeDef:   type definition of the symbol
//              (used for variables and functions)
//              (a vector because functions can have multiple args)
// - isDefined: whether the symbol is defined or not
// - value:     value of the symbol (used for variables)

struct symbolInfo {
  std::string name;
  linkage link;
  symbolType type;
  std::vector<ast::ElemType> typeDef;
  std::map<int, derivedSymbol> derivedTypeMap;
  defType def = defType::FALSE;
  constant::Constant value{};
};
} // namespace symbolTable
} // namespace scarlet
