#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include "../../MyLib/Basic/Value.h"
#include "OperationalLogic.h"

class OpCreator {
private:
    const std::string OpCode_Assign = "ASN";
    const std::string OpCode_Add = "ADD";
    const std::string OpCode_Subtract = "SUB";
    const std::string OpCode_Multiply = "MUL";
    const std::string OpCode_Divide = "DIV";
    const std::string OpCode_ShiftLeft = "SLF";
    const std::string OpCode_ShiftRightSigned = "SRS";
    const std::string OpCode_ShiftRightUnsigned = "SRU";
    const std::string OpCode_And = "AND";
    const std::string OpCode_Or = "OR";
    const std::string OpCode_Xor = "XOR";
    const std::string OpCode_JumpEqual = "JEQ";
    const std::string OpCode_JumpNotEqual = "JNE";
    const std::string OpCode_JumpGreater = "JGT";
    const std::string OpCode_JumpGreaterEqual = "JGE";
    const std::string OpCode_JumpLessThan = "JLT";
    const std::string OpCode_JumpLessOrEqual = "JLE";
    const std::string OpCode_Return = "RTN";
    const std::string VariableName_Input = "input";

public:
    OpCreator();
    ~OpCreator();
    std::shared_ptr<OperationalLogic> load_logic(const std::string& logic);
    void clear();

private:
    bool add_instruction(const std::string& sOpCode, const std::string& dest, const std::string& r1, const std::string& r2);

    struct ValueData {
        bool isConstant;
        Value constantValue;
    };
    struct InstructionData {
        OperationalLogic::OpCode opCode;
        size_t destIndex;
        std::pair<bool, size_t> v1Id;
        std::pair<bool, size_t> v2Id;
    };
    OperationalLogic::OpCode get_opcode(const std::string& sOpCode) const;
    std::pair<bool, size_t> get_variable(const std::string& value);
    Value get_constant_value(const std::string& value) const;
    size_t get_volatile_value_index(const std::string& name);

private:
    size_t volatileValueCount;
    //std::vector<std::string> volatileValue;
    std::unordered_map<std::string, size_t> name2VolatileValueIndex;
    std::vector<Value> constantValue;
    std::vector<InstructionData> instructionList;
};