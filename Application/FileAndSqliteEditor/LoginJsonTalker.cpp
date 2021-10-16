#include "LoginJsonTalker.h"
#include <iostream>

using namespace std;

LoginJsonTalker::LoginJsonTalker(const std::string& dbName) : theDb(dbName) {

}

LoginJsonTalker::~LoginJsonTalker() {

}

string LoginJsonTalker::process_check_match(const nlohmann::json& input) {
    nlohmann::json theReply;
    if(!input.contains(Property_Username) || !input.contains(Property_Password)) {
        theReply["Status"] = "Bad";
        theReply["Message"] = "Check Match requires both username and password";
        return theReply.dump();
    }
    const nlohmann::json& jUsername = input[Property_Username];
    const nlohmann::json& jPassword = input[Property_Password];
    if(!jUsername.is_string() || !jPassword.is_string()) {
        theReply["Status"] = "Bad";
        theReply["Message"] = "Both username and passowr must be string";
        return theReply.dump();
    }
    string sUsername = jUsername.get<string>();
    string sPassword = jPassword.get<string>();
    string statement = "Select Count(*) from UserAccount where Username=='";
    statement += sUsername;
    statement += "' AND Password=='";
    statement += sPassword;
    statement += "'";
    try {
        auto result = theDb.execute_query(statement);
        theReply["Status"] = "Good";
        if(result->get_integer(0, 0).second) {
            theReply["Match"] = true;
        }
        else {
            theReply["Match"] = false;
        }
        return theReply.dump();
    }catch(Sqlite3::Exception e) {
		theReply["Status"] = "Bad";
		theReply["Message"] = e.message;
        return theReply.dump();
    }
}

string LoginJsonTalker::process_change_username(const nlohmann::json& input) {
    nlohmann::json theReply;
    if(!input.contains(Property_Username) || !input.contains(Property_Password)) {
        theReply["Status"] = "Bad";
        theReply["Message"] = "Change username requires username and password";
        return theReply.dump();
    }
    auto jUsername = input[Property_Username];
    auto jPassword = input[Property_Password];
    if(!jUsername.is_string() || !jPassword.is_string()) {
        theReply["Status"] = "Bad";
        theReply["Message"] = "username, password and new password must be string";
        return theReply.dump();
    }
    string sUsername = jUsername.get<string>();
    string sPassword = jPassword.get<string>();
    string statement = "Update UserAccount set Username='";
    statement += sUsername;
    statement += "' where Password=='";
    statement += sPassword;
    statement += "';Select count(*) from UserAccount where Username=='";
    statement += sUsername;
    statement += "'";
    try {
        cout<<statement<<endl;
        auto result = theDb.execute_query(statement);
        if(result->get_integer(0, 0).second) {
            theReply["Status"] = "Good";
        }
        else {
            theReply["Status"] = "Bad";
            theReply["Message"] = "Wrong password";
        }
        return theReply.dump();
    }catch(Sqlite3::Exception e) {
		theReply["Status"] = "Bad";
		theReply["Message"] = e.message;
        return theReply.dump();
    }
}

string LoginJsonTalker::process_change_password(const nlohmann::json& input) {
    nlohmann::json theReply;
    if(!input.contains(Property_Password) || !input.contains(Property_NewPassword)) {
        theReply["Status"] = "Bad";
        theReply["Message"] = "Change password requires password and new password";
        return theReply.dump();
    }
    auto jPassword = input[Property_Password];
    auto jNewPassword = input[Property_NewPassword];
    if(!jPassword.is_string() || !jNewPassword.is_string()) {
        theReply["Status"] = "Bad";
        theReply["Message"] = "password and new password must be string";
        return theReply.dump();
    }
    string sPassword = jPassword.get<string>();
    string sNewPassword = jNewPassword.get<string>();
    string statement = "Update UserAccount set Password='";
    statement += sNewPassword;
    statement += "' where Password=='";
    statement += sPassword;
    statement += "';Select count(*) from UserAccount where Password=='";
    statement += sNewPassword;
    statement += "'";
    try {
        cout<<statement<<endl;
        auto result = theDb.execute_query(statement);
        if(result->get_integer(0, 0).second) {
            theReply["Status"] = "Good";
        }
        else {
            theReply["Status"] = "Bad";
            theReply["Message"] = "Wrong password";
        }
        return theReply.dump();
    }catch(Sqlite3::Exception e) {
		theReply["Status"] = "Bad";
		theReply["Message"] = e.message;
        return theReply.dump();
    }
}

string LoginJsonTalker::parse_command(const string& input) {
    nlohmann::json theReply;
    nlohmann::json theJson;
    try {
        theJson = nlohmann::json::parse(input);
    } catch(nlohmann::json::parse_error& error) {
		printf("Parse error: %s.\n", input.c_str());
        theReply["Status"] = "Bad";
		theReply["Message"] = "Json parse error";
		return theReply.dump() + '\n';
    }
    if(!theJson.is_object()) {
		theReply["Status"] = "Bad";
		theReply["Message"] = "Frame is not a json object";
		return theReply.dump() + '\n';
    }
	if (!theJson.contains(Property_Command)) {
		theReply["Status"] = "Bad";
		theReply["Message"] = "No command";
		return theReply.dump() + '\n';
	}
    nlohmann::json& jCommand = theJson[Property_Command];
    if(!jCommand.is_string()) {
		theReply["Status"] = "Bad";
		theReply["Message"] = "Command should be string type";
		return theReply.dump() + '\n';
    }
    string command = jCommand.get<string>();
    if(!command.compare(Command_CheckMatch)) {
        return this->process_check_match(theJson);
    }
    else if(!command.compare(Command_ChangeUsername)) {
        return this->process_change_username(theJson);
    }
    else if(!command.compare(Command_ChangePassword)) {
        return this->process_change_password(theJson);
    }
	theReply["Status"] = "Bad";
	theReply["Message"] = "Unknown command";
	return theReply.dump() + '\n';
}
