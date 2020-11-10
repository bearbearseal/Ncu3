#include "OperationalLogic.h"

using namespace std;

OperationalLogic::OperationalLogic(const std::vector<Value>& _constantValue, size_t _volatileCount) {
    constantValue = _constantValue;
    volatileCount = _volatileCount;

}

OperationalLogic::~OperationalLogic() {

}

void OperationalLogic::add_logic(OpCode opCode, size_t dIndex, std::pair<size_t, bool> v1, std::pair<size_t, bool> v2) {
    switch(opCode) {
        case OpCode::Assign:
        case OpCode::Add:
        case OpCode::Subtract:
        case OpCode::Multiply:
        case OpCode::Divide:
        case OpCode::ShiftLeft:
        case OpCode::ShiftRightSigned:
        case OpCode::ShiftRightUnsigned:
        case OpCode::And:
        case OpCode::Or:
        case OpCode::Xor:
            instructionList.push_back({InstructionType::Operation, Operation(opCode, ValueId{v1.second, v1.first}, ValueId{v2.second, v2.first}), dIndex});
            break;
        case OpCode::Jump:
        case OpCode::JumpEqual:
        case OpCode::JumpNotEqual:
        case OpCode::JumpGreater:
        case OpCode::JumpGreaterEqual:
        case OpCode::JumpLess:
        case OpCode::JumpLessEqual:
            instructionList.push_back({InstructionType::Jump, Jump(opCode, ValueId{v1.second, v1.first}, ValueId{v2.second, v2.first}), dIndex});
            break;
        case OpCode::Return:
            instructionList.push_back({InstructionType::Return, false, dIndex});
            break;
    }
}

Value OperationalLogic::execute() {
    vector<Value> volatileValue;
    volatileValue.resize(volatileCount);
    size_t instructionDone = 0;
    for(size_t i=0; i<instructionList.size();) {
        if(instructionList[i].type == InstructionType::Jump) {
            const Jump& jumpInstruction = std::get<Jump>(instructionList[i].instruction);
            if(jumpInstruction.execute(volatileValue, constantValue)) {
                //jump
                i = instructionList[i].dIndex;
            }
            else {
                ++i;
            }
            break;
        }
        else if(instructionList[i].type == InstructionType::Operation) {
            const Operation& operationInstruction = std::get<Operation>(instructionList[i].instruction);
            volatileValue[instructionList[i].dIndex] = operationInstruction.execute(volatileValue, constantValue);
            ++i;
        }
        else {
            return volatileValue[instructionList[i].dIndex];
        }
        ++instructionDone;
        if(instructionDone >= MaxInstruction) {
            return Value();
        }
    }
    return volatileValue[0];
}

OperationalLogic::Operation::Operation(OpCode _opCode, const ValueId& _v1Id, const ValueId& _v2Id) : opCode(_opCode), v1Id(_v1Id), v2Id(_v2Id) {

}

OperationalLogic::Operation::~Operation() {

}

Value OperationalLogic::Operation::execute(const std::vector<Value>& volatileValue, const std::vector<Value>& constantValue) const {
    const Value *v1, *v2;
    if(v1Id.isConstant) {
        v1 = &constantValue[v1Id.index];
    }
    else {
        v1 = &volatileValue[v1Id.index];
    }
    if(v2Id.isConstant) {
        v2 = &constantValue[v2Id.index];
    }
    else {
        v2 = &volatileValue[v2Id.index];
    }
    switch(opCode) {
        case OpCode::Assign:
            return *v1;
        case OpCode::Add:
            return *v1 + *v2;
        case OpCode::Subtract:
            return *v1 - *v2;
        case OpCode::Multiply:
            return *v1 * *v2;
        case OpCode::Divide:
            return *v1 / *v2;
        case OpCode::ShiftLeft:
            return v1->get_int() << v2->get_int();
        case OpCode::ShiftRightSigned:
            return v1->get_int() >> v2->get_int();
        case OpCode::ShiftRightUnsigned:
            return uint64_t(v1->get_int()) >> v2->get_int();
        case OpCode::And:
            return v1->get_int() & v2->get_int();
        case OpCode::Or:
            return v1->get_int() | v2->get_int();
        case OpCode::Xor:
            return v1->get_int() ^ v2->get_int();
        default:
            return Value();
    }
}

OperationalLogic::Jump::Jump(OpCode _opCode, const ValueId& _v1Id, const ValueId& _v2Id) : opCode(_opCode), v1Id(_v1Id), v2Id(_v2Id) {

}

OperationalLogic::Jump::~Jump() {

}

bool OperationalLogic::Jump::execute(const std::vector<Value>& volatileValue, const std::vector<Value>& constantValue) const {
    const Value *v1, *v2;
    if(v1Id.isConstant) {
        v1 = &constantValue[v1Id.index];
    }
    else {
        v1 = &volatileValue[v1Id.index];
    }
    if(v2Id.isConstant) {
        v2 = &constantValue[v2Id.index];
    }
    else {
        v2 = &volatileValue[v2Id.index];
    }
    switch(opCode) {
        case OpCode::Jump:
            return true;
        case OpCode::JumpEqual:
            return *v1 == *v2;
        case OpCode::JumpNotEqual:
            return *v1 != *v2;
        case OpCode::JumpGreater:
            return *v1 > *v2;
        case OpCode::JumpGreaterEqual:
            return *v1 >= *v2;
        case OpCode::JumpLess:
            return *v1 < *v2;
        case OpCode::JumpLessEqual:
            return *v1 <= *v2;
        default:
            return false;
    }
}
