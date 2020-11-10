#pragma once
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <utility>
#include "../MyLib/Basic/Value.h"
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

public:
    OpCreator();
    ~OpCreator();
    bool add_operation(const std::string& sOpCode, const std::string& r1, const std::string& r2, const std::string& dest);
    void clear();

private:
    struct ValueData {
        bool isConstant;
        Value constantValue;
        size_t volatileValueIndex;
    };
    OperationalLogic::OpCode get_opcode(const std::string& sOpCode);
    ValueData get_value(const std::string& value);
    size_t get_dest_index(const std::string& dest);

private:
    std::unordered_set<std::string> variableNames;
    std::unordered_map<std::string, size_t> valueName2Index;
    std::vector<Value> constantValue;
};