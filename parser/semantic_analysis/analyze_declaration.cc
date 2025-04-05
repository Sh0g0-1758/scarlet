#include <parser/common.hh>

namespace scarlet {
namespace parser {

void parser::analyze_declaration(
    std::shared_ptr<ast::AST_Declaration_Node> declaration,
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        &symbol_table,
    int indx) {
  std::string var_name = declaration->get_identifier()->get_value();
  if (declaration->get_type() == ast::DeclarationType::VARIABLE) {
    if (symbol_table.find({var_name, indx}) != symbol_table.end()) {
      // the symbol has been declared twice which is illegal
      // but if both the declarations are extern then it is legal
      if (symbol_table[{var_name, indx}].link ==
              symbolTable::linkage::EXTERNAL and
          declaration->get_specifier() == ast::SpecifierType::EXTERN) {
        auto varDecl =
            std::static_pointer_cast<ast::AST_variable_declaration_Node>(
                declaration);
        analyze_local_variable_declaration(varDecl, symbol_table, var_name,
                                           indx);
      } else {
        success = false;
        error_messages.emplace_back("Variable " + var_name +
                                    " already declared");
      }
    } else {
      auto varDecl =
          std::static_pointer_cast<ast::AST_variable_declaration_Node>(
              declaration);
      analyze_local_variable_declaration(varDecl, symbol_table, var_name, indx);
    }
  } else {
    // Make sure that function declarations at block scope don't have
    // static storage specifier.
    if (indx != 0 and
        declaration->get_specifier() == ast::SpecifierType::STATIC) {
      success = false;
      error_messages.emplace_back(
          "Function " + var_name +
          " declared with static storage specifier inside a block");
    }
    auto funcDecl =
        std::static_pointer_cast<ast::AST_function_declaration_Node>(
            declaration);
    analyze_function_declaration(funcDecl, symbol_table, var_name, indx);
  }
}

void parser::unroll_derived_type(
    std::shared_ptr<ast::AST_declarator_Node> declarator,
    std::vector<long> &derivedType) {
  if (declarator == nullptr)
    return;

  unroll_derived_type(declarator->get_child(), derivedType);

  if (declarator->is_pointer()) {
    derivedType.push_back((long)ast::ElemType::POINTER);
  }

  if (!declarator->get_arrDim().empty()) {
    for (auto dim : declarator->get_arrDim()) {
      derivedType.push_back(dim);
    }
  }
}

bool parser::previous_declaration_has_same_type(
    ast::ElemType prev_base_type, std::vector<long> prev_derived_type,
    std::shared_ptr<ast::AST_declarator_Node> curr_declarator,
    ast::ElemType curr_base_type) {
  std::vector<long> derivedType;
  unroll_derived_type(curr_declarator, derivedType);
  if (!derivedType.empty()) {
    derivedType.push_back((long)curr_base_type);
    curr_base_type = ast::ElemType::DERIVED;
  }

  if ((prev_base_type != curr_base_type) or (prev_derived_type != derivedType))
    return false;
  return true;
}

} // namespace parser
} // namespace scarlet
