#include "OpCreator.h"

using namespace std;

OpCreator::OpCreator() {

}

OpCreator::~OpCreator() {

}

bool OpCreator::add_operation(const std::string& sOpCode, const std::string& r1, const std::string& r2, const std::string& dest) {
    try{
        OperationalLogic::OpCode opCode = get_opcode(sOpCode);
    }catch(int errorNum) {
        return false;
    }
    return true;
}

void OpCreator::clear() {

}

OperationalLogic::OpCode OpCreator::get_opcode(const std::string& sOpCode) {
    if(!sOpCode.compare(OpCode_Assign)) {
        return OperationalLogic::OpCode::Assign;
    }
    else if(!sOpCode.compare(OpCode_Add)) {
        return OperationalLogic::OpCode::Add;
    }
    else if(!sOpCode.compare(OpCode_Subtract)) {
        return OperationalLogic::OpCode::Subtract;
    }
    else if(!sOpCode.compare(OpCode_Multiply)) {
        return OperationalLogic::OpCode::Multiply;
    }
    else if(!sOpCode.compare(OpCode_Divide)) {
        return OperationalLogic::OpCode::Divide;
    }
    else if(!sOpCode.compare(OpCode_ShiftLeft)) {
        return OperationalLogic::OpCode::ShiftLeft;
    }
    else if(!sOpCode.compare(OpCode_ShiftRightSigned)) {
        return OperationalLogic::OpCode::ShiftRightSigned;
    }
    else if(!sOpCode.compare(OpCode_ShiftRightUnsigned)) {
        return OperationalLogic::OpCode::ShiftRightUnsigned;
    }
    else if(!sOpCode.compare(OpCode_And)) {
        return OperationalLogic::OpCode::And;
    }
    else if(!sOpCode.compare(OpCode_Or)) {
        return OperationalLogic::OpCode::Or;
    }
    else if(!sOpCode.compare(OpCode_Xor)) {
        return OperationalLogic::OpCode::Xor;
    }
    else if(!sOpCode.compare(OpCode_JumpEqual)) {
        return OperationalLogic::OpCode::JumpEqual;
    }
    else if(!sOpCode.compare(OpCode_JumpNotEqual)) {
        return OperationalLogic::OpCode::JumpNotEqual;
    }
    else if(!sOpCode.compare(OpCode_JumpGreater)) {
        return OperationalLogic::OpCode::JumpGreater;
    }
    else if(!sOpCode.compare(OpCode_JumpGreaterEqual)) {
        return OperationalLogic::OpCode::JumpGreaterEqual;
    }
    else if(!sOpCode.compare(OpCode_JumpLessThan)) {
        return OperationalLogic::OpCode::JumpLess;
    }
    else if(!sOpCode.compare(OpCode_JumpLessOrEqual)) {
        return OperationalLogic::OpCode::JumpLessEqual;
    }
    else if(!sOpCode.compare(OpCode_Return)) {
        return OperationalLogic::OpCode::Return;
    }
    throw 100;
}

OpCreator::ValueData OpCreator::get_value(const std::string& value) {

}

size_t OpCreator::get_dest_index(const std::string& dest) {

}
