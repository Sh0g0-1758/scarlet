#pragma once

#include <ast/ast.hh>
#include <string>
#include <tools/constant/constant.hh>
#include <variant>
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
  std::vector<ast::ElemType> typeDef{};
  defType def = defType::FALSE;
  std::vector<constant::Constant> value{};
  bool is_variadic_func = false;
  // Here <= 0 corresponds to an ElemType and > 0 corrresponds to an array
  // dimension
  std::map<int, std::vector<long>> derivedTypeMap{};

  static bool is_array(symbolInfo symInfo) {
    return symInfo.typeDef[0] == ast::ElemType::DERIVED and
           symInfo.derivedTypeMap[0][0] > 0;
  }
};

} // namespace symbolTable
} // namespace scarlet
