#include "codegen.hh"

void Codegen::codegen() {
    std::string assembly;
    for(auto it : program.get_functions()) {
        assembly += "\t.globl ";
        assembly += it.get_identifier().get_value() + "\n";
        assembly += it.get_identifier().get_value() + ":\n";
        for(auto inst : it.get_statements()) {
            if(inst.get_type() == "Return") {
                assembly += "\tmovl ";
                assembly += "$" + inst.get_exps()[0].get_int_node().get_value() + ", %eax\n";
                assembly += "\tret\n";
            }
        }
    }

    assembly += "\t.section    .note.GNU-stack,\"\",@progbits\n";

    std::ofstream file(file_name);
    if(file.is_open()) {
        file << assembly;
        file.close();
    } else {
        std::cerr << "[ERROR]: Unable to open file" << std::endl;
        success = false;
    }
}
