#pragma once
#include "../../MyLib/Basic/Value.h"
#include <memory>
#include <vector>
#include <utility>
#include <variant>

class OperationalLogic {
    friend class OpCreator;
private:
    const size_t MaxInstruction = 128;
 
public:
    enum class OpCode {
        Assign              =0x01,
        Add                 =0x02,
        Subtract            =0x03,
        Multiply            =0x04,
        Divide              =0x05,
        ShiftLeft           =0x06,
        ShiftRightSigned    =0x07,
        ShiftRightUnsigned  =0x08,
        And                 =0x09,
        Or                  =0x0A,
        Xor                 =0x0B,
        JumpEqual           =0x101,
        JumpNotEqual        =0x102,
        JumpGreater         =0x103,
        JumpGreaterEqual    =0x104,
        JumpLess            =0x105,
        JumpLessEqual       =0x106,
        Jump                =0x107,
        Return              =0x200
    };
    OperationalLogic(const std::vector<Value>& _constantValue, size_t _volatileCount);
    ~OperationalLogic();
    Value execute(const Value& value);
    
private:
    //bool add_logic(OpCode opCode, size_t dIndex, size_t v1Index, bool v1isConstant, size_t v2Index, bool v2isConstant);
    void add_logic(OpCode opCode, size_t dest, std::pair<bool, size_t> v1, std::pair<bool, size_t> v2);
    static bool is_operation_type(OpCode opCode) {return (size_t(opCode) & 0xff00) == 0;}
    static bool is_jump_type(OpCode opCode) {return (size_t(opCode) & 0xff00) == 0x100;}
    static bool is_return_type(OpCode opCode) { return (size_t(opCode) & 0xff00) == 0x200;}

private:
    enum class InstructionType {
        Return,
        Jump,
        Operation
    };
    struct ValueId {
        bool isConstant;
        size_t index;
    };
    class Operation {
    public:
        Operation(OpCode _opCode, const ValueId& _v1Id, const ValueId& _v2Id);
        ~Operation();
        Value execute(const std::vector<Value>& volatileValue, const std::vector<Value>& constantValue) const;
    private:
        const OpCode opCode;
        const ValueId v1Id;
        const ValueId v2Id;
    };
    class Jump {
    public:
        Jump(OpCode _opCode, const ValueId& _v1Id, const ValueId& _v2Id);
        ~Jump();
        bool execute(const std::vector<Value>& volatileValue, const std::vector<Value>& constantValue) const;
    private:
        const OpCode opCode;
        const ValueId v1Id;
        const ValueId v2Id;
    };
    struct Instruction {
        InstructionType type;
        std::variant<Operation, Jump, bool> instruction;
        size_t index;   //volatileValue index or jump index;
    };
    size_t volatileCount = 0;
    std::vector<Value> constantValue;
    std::vector<Instruction> instructionList;
};