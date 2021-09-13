#include "OpCreator.h"
#include "../../MyLib/StringManipulator/StringManipulator.h"

using namespace std;

OpCreator::OpCreator() {
    name2VolatileValueIndex.emplace(VariableName_Input, 0);
    volatileValueCount = 1;
}

OpCreator::~OpCreator() {

}

bool string_is_variable(const std::string& rawString) {
    if(!rawString.size()) {
        return false;
    }
    if((rawString.front() < 'A' || rawString.front() > 'Z') && (rawString.front() < 'a' || rawString.front() > 'z') && rawString.front() != '_') {
        return false;
    }
    for(size_t i=1; i<rawString.size(); ++i) {
        if((rawString[i] < 'A' || rawString[i] > 'Z') && (rawString[i] < 'a' || rawString[i] > 'z') && rawString[i] != '_' && (rawString[i] < '0' || rawString[i] > '9') && rawString[i] != '@') {
            return false;
        }
    }
    return true;
}

bool string_is_string(const std::string& rawString) {
    if(rawString.size()<3) {
        return false;
    }
    if(rawString.front() == '\"' && rawString.back() == '\"') {
        return true;
    }
    return false;
}

bool string_is_integer(const std::string& rawString) {
    if(!rawString.size()) {
        return false;
    }
    if(rawString.front() != '-' && (rawString.front() < '0' || rawString.front() > '9')) {
        return false;
    }
    if(rawString.front() == '-' && rawString.size()<2) {
        return false;
    }
    for(size_t i=1; i<rawString.size(); ++i) {
        if((rawString[i] < '0' || rawString[i] > '9')) {
            return false;
        }
    }
    return true;
}

bool string_is_float(const std::string& rawString) {
    if(!rawString.size()) {
        return false;
    }
    if((rawString.front() < '0' || rawString.front() > '9') && rawString.front() != '-') {
        return false;
    }
    if(rawString.back() < '0' || rawString.back() > '9') {
        return false;
    }
    if(rawString.front() == '-' && rawString.size()<2) {
        return false;
    }
    bool hasDecimal = false;
    for(size_t i=1; i<rawString.size(); ++i) {
        if(rawString[i] == '.') {
            if(hasDecimal) {
                return false;
            }
            hasDecimal = true;
        }
        else if(rawString[i] < '0' || rawString[i] > '9') {
            return false;
        }
    }
    if(hasDecimal) {
        return true;
    }
    return false;
}

shared_ptr<OperationalLogic> OpCreator::load_logic(const string& logic) {
    StringManipulator::Tokenizer outerTokenizer, innerTokenizer;
    outerTokenizer.add_deliminator('\n');
    innerTokenizer.add_deliminator('\t');
    innerTokenizer.add_deliminator(' ');
    innerTokenizer.add_wrapper('\"', '\"');
    auto tokens = outerTokenizer.tokenize_to_vector(logic);
    for(size_t i=0; i<tokens.size(); ++i) {
        auto subTokens = innerTokenizer.tokenize_to_vector(tokens[i]);
        if(subTokens.size() > 4) {
            //printf("More than 4 tokens.\n");
            return nullptr;
        }
        subTokens.resize(4);
        if(!add_instruction(subTokens[0], subTokens[1], subTokens[2], subTokens[3])) {
            //printf("Add failed %s.\n", subTokens[0].c_str());
            return nullptr;
        }
    }
    //printf("Volatile value count now: %lu\n", volatileValueCount);
    shared_ptr<OperationalLogic> retVal = make_shared<OperationalLogic>(constantValue, volatileValueCount);
    for(size_t i=0; i<instructionList.size(); ++i) {
        retVal->add_logic(instructionList[i].opCode, instructionList[i].destIndex, instructionList[i].v1Id, instructionList[i].v2Id);
    }
    return retVal;
}

bool OpCreator::add_instruction(const std::string& sOpCode, const std::string& dest, const std::string& r1, const std::string& r2) {
    InstructionData instructionData;
    try{
        instructionData.opCode = get_opcode(sOpCode);
        if(OperationalLogic::is_jump_type(instructionData.opCode)) {
            //dest has to be constant integer
            if(!string_is_integer(dest)) {
                return false;
            }
            else {
                int64_t destValue = stoll(dest);
                //line number starts at 1, so offset by -1
                if(destValue<1) {
                    return false;
                }
                instructionData.destIndex = destValue - 1;
                //constantValue.push_back(destValue);
            }
            //need r1, r2 if not direct jump
            if(instructionData.opCode != OperationalLogic::OpCode::Jump) {
                instructionData.v1Id = get_variable(r1);
                instructionData.v2Id = get_variable(r2);
            }
        }
        else if(OperationalLogic::is_operation_type(instructionData.opCode)) {
            //dest has to be volatile value
            if(!string_is_variable(dest)) {
                return false;
            }
            instructionData.destIndex = get_volatile_value_index(dest);
            instructionData.v1Id = get_variable(r1);
            //if assign then only need r1
            if(instructionData.opCode != OperationalLogic::OpCode::Assign) {
                instructionData.v2Id = get_variable(r2);
            }
        }
        else if(OperationalLogic::is_return_type(instructionData.opCode)) {
            if(!string_is_variable(dest)) {
                Value destValue = get_constant_value(dest);
                instructionData.destIndex = constantValue.size();
                constantValue.push_back(destValue);
            }
            else {
                instructionData.destIndex = get_volatile_value_index(dest);
            }
        }
        instructionList.push_back(instructionData);
    }catch(int errorNum) {
        printf("Exception number: %d\n", errorNum);
        return false;
    }
    return true;
}

void OpCreator::clear() {
    volatileValueCount = 0;
    name2VolatileValueIndex.clear();
    constantValue.clear();
    instructionList.clear();
    name2VolatileValueIndex.emplace(VariableName_Input, 0);
    volatileValueCount = 1;
}

OperationalLogic::OpCode OpCreator::get_opcode(const std::string& sOpCode) const {
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

std::pair<bool, size_t> OpCreator::get_variable(const std::string& rawString) {
    if(string_is_variable(rawString)) {
        auto variableIndex = get_volatile_value_index(rawString);
        return {false, variableIndex};
    }
    else {
        Value value = get_constant_value(rawString);
        if(value.is_empty()) {
            throw 200;
        }
        size_t variableIndex = constantValue.size();
        constantValue.push_back(value);
        return {true, variableIndex};
    }
}

Value OpCreator::get_constant_value(const std::string& value) const {
    if(string_is_float(value)) {
        return stod(value);
    }
    else if(string_is_integer(value)) {
        return int64_t(stoll(value));
    }
    else if(string_is_string(value)) {
        return value.substr(1, value.size()-2);
    }
    return Value();
}

size_t OpCreator::get_volatile_value_index(const std::string& name) {
    size_t index;
    auto i = name2VolatileValueIndex.find(name);
    if(i == name2VolatileValueIndex.end()) {
        index = volatileValueCount;
        name2VolatileValueIndex.emplace(name, volatileValueCount);
        ++volatileValueCount;
    }
    else {
        index = i->second;
    }
    return index;
}
