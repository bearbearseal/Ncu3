#include "OperationalLogic.h"

using namespace std;

OperationalLogic::OperationalLogic(const std::vector<Value>& _constantValue, size_t _volatileCount) {
    constantValue = _constantValue;
    volatileCount = _volatileCount;
    printf("Volatile count: %lu.\n", volatileCount);
}

OperationalLogic::~OperationalLogic() {

}

void OperationalLogic::add_logic(OpCode opCode, size_t dest, std::pair<bool, size_t> v1, std::pair<bool, size_t> v2) {
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
            instructionList.push_back({InstructionType::Operation, Operation(opCode, ValueId{v1.first, v1.second}, ValueId{v2.first, v2.second}), dest});
            break;
        case OpCode::JumpEqual:
        case OpCode::JumpNotEqual:
        case OpCode::JumpGreater:
        case OpCode::JumpGreaterEqual:
        case OpCode::JumpLess:
        case OpCode::JumpLessEqual:
        case OpCode::Jump:
            instructionList.push_back({InstructionType::Jump, Jump(opCode, ValueId{v1.first, v1.second}, ValueId{v2.first, v2.second}), dest});
            break;
        case OpCode::Return:
            instructionList.push_back({InstructionType::Return, false, dest});
            break;
    }
}

Value OperationalLogic::execute(const Value& value) {
    vector<Value> volatileValue;
    volatileValue.resize(volatileCount);
    volatileValue[0] = value;
    size_t instructionDone = 0;
    for(size_t i=0; i<instructionList.size();) {
        if(instructionList[i].type == InstructionType::Jump) {
            printf("Executing Jump type.\n");
            const Jump& jumpInstruction = std::get<Jump>(instructionList[i].instruction);
            if(jumpInstruction.execute(volatileValue, constantValue)) {
                //jump
                printf("Jumping to %lu\n", instructionList[i].index);
                i = instructionList[i].index;
            }
            else {
                printf("No jump.\n");
                ++i;
            }
        }
        else if(instructionList[i].type == InstructionType::Operation) {
            printf("Executing operation type.\n");
            const Operation& operationInstruction = std::get<Operation>(instructionList[i].instruction);
            printf("Dest Index: %lu\n", instructionList[i].index);
            volatileValue[instructionList[i].index] = operationInstruction.execute(volatileValue, constantValue);
            printf("Value after operation: %s\n", volatileValue[instructionList[i].index].to_string().c_str());
            ++i;
        }
        else {
            printf("Executing return type.\n");
            printf("Return index: %lu\n", instructionList[i].index);
            return volatileValue[instructionList[i].index];
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
        case OpCode::Jump:
            return true;
        default:
            return false;
    }
}
