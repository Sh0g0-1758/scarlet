#include <parser/common.hh>

namespace scarlet {
namespace parser {
    void parser::analyze_global_struct_declaration(
    std::shared_ptr<ast::AST_struct_declaration_Node> structDecl,
    std::map<std::pair<std::string, int>, symbolTable::symbolInfo>
        &struct_symbol_table){
    std::string struct_name = structDecl->get_identifier()->get_value();
    if(globalStructSymbolTable.find(struct_name) != globalStructSymbolTable.end() 
        and globalStructSymbolTable[struct_name].def == symbolTable::defType::TRUE
        and (!structDecl->get_members().empty())) {
        success = false;
        error_messages.emplace_back("Struct " + struct_name +
                                    " has already been defined");
    }
    struct_symbol_table[{struct_name, 0}].def = symbolTable::defType::TRUE;
    if(globalStructSymbolTable.find(struct_name) == globalStructSymbolTable.end()){
        symbolTable::symbolInfo structInfo;
        structInfo.name = struct_name;
        structInfo.link = symbolTable::linkage::NONE;
        structInfo.type = symbolTable::symbolType::STRUCT;
        structInfo.def = symbolTable::defType::FALSE;
        globalStructSymbolTable[struct_name] = structInfo;
    }
    std::set<std::string> memNames;
    if(!structDecl->get_members().empty()){
        for(auto member : structDecl->get_members()){
            std::string mem_name = member->get_identifier()->get_value();
            if(memNames.find(mem_name) != memNames.end()){
                success = false;
                error_messages.emplace_back("Struct " + struct_name +
                                            " has duplicate member " + mem_name);
            }else{
                memNames.insert(mem_name);
                globalStructSymbolTable[struct_name].memNametoIndex[mem_name] = globalStructSymbolTable[struct_name].struct_type.size();
            }
            symbolTable::symbolInfo memInfo;
            memInfo.name = mem_name;
            memInfo.link = symbolTable::linkage::NONE;
            memInfo.type = symbolTable::symbolType::VARIABLE;
            memInfo.def = symbolTable::defType::TRUE;
            std::vector<long> derivedType;
            ast::unroll_derived_type(member->get_declarator(), derivedType);
            if(member->get_Decltype() == ast::DeclarationType::STRUCT){
                if(globalStructSymbolTable.find(member->get_struct_identifier()->get_value()) == globalStructSymbolTable.end()
                    or globalStructSymbolTable[member->get_struct_identifier()->get_value()].def == symbolTable::defType::FALSE){
                    if(derivedType[derivedType.size()-1] != (long)ast::ElemType::POINTER){
                    success = false;
                    error_messages.emplace_back("Struct " + member->get_struct_identifier()->get_value() +
                                                " has not been defined");
                    }
                }
                memInfo.type = symbolTable::symbolType::STRUCT;
                constant::Constant constVal;
                constVal.set_string(member->get_struct_identifier()->get_value());
                memInfo.struct_identifier = constVal;
            }
            if (!derivedType.empty()) {
            derivedType.push_back((long)member->get_base_type());
            memInfo.typeDef.push_back(ast::ElemType::DERIVED);
            memInfo.derivedTypeMap[0] = derivedType;
            } else {
            if(member->get_Decltype() == ast::DeclarationType::STRUCT){
                if(memInfo.struct_identifier.get_string() == struct_name){
                    success = false;
                    error_messages.emplace_back("Struct " + struct_name +
                                                " cannot contain itself");
                }
            }
            memInfo.typeDef.push_back(member->get_base_type());
            }

            if (!ast::validate_type_specifier(memInfo.typeDef[0],
                                            memInfo.derivedTypeMap[0])) {
            success = false;
            error_messages.emplace_back("Variable " + mem_name +
                                        " cannot be declared as incomplete type");
            }
            globalStructSymbolTable[struct_name].struct_type.emplace_back(memInfo);     
        }
        globalStructSymbolTable[struct_name].def = symbolTable::defType::TRUE;
    }

}
}
}