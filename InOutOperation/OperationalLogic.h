#pragma once
#include "../../MyLib/Basic/Value.h"
#include <memory>
#include <vector>
#include <utility>
#include <variant>

class OperationalLogic {
    friend class OperationLogic;
private:
    const size_t MaxInstruction = 128;
 
public:
    enum class OpCode {
        Assign,
        Add,
        Subtract,
        Multiply,
        Divide,
        ShiftLeft,
        ShiftRightSigned,
        ShiftRightUnsigned,
        And,
        Or,
        Xor,
        Jump,
        JumpEqual,
        JumpNotEqual,
        JumpGreater,
        JumpGreaterEqual,
        JumpLess,
        JumpLessEqual,
        Return
    };
    OperationalLogic(const std::vector<Value>& _constantValue, size_t _volatileCount);
    ~OperationalLogic();
    Value execute();
    
private:
    //bool add_logic(OpCode opCode, size_t dIndex, size_t v1Index, bool v1isConstant, size_t v2Index, bool v2isConstant);
    void add_logic(OpCode opCode, size_t dIndex, std::pair<size_t, bool> v1, std::pair<size_t, bool> v2);

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
        size_t dIndex;
    };
    size_t volatileCount;
    std::vector<Value> constantValue;
    std::vector<Instruction> instructionList;
};