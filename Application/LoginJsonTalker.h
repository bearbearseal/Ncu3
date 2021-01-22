#ifndef _LoginJsonTalker_H_
#define _LoginJsonTalker_H_
#include <string>
#include "../../MyLib/Sqlite/Sqlite3.h"
#include "../../OtherLib/nlohmann/json.hpp"

class LoginJsonTalker {
public:
    const std::string Command_CheckMatch = "CheckMatch";
    const std::string Command_ChangeUsername = "ChangeUsername";
    const std::string Command_ChangePassword = "ChangePassword";
    const std::string Property_Statement = "Statement";
    const std::string Property_Command = "Command";
    const std::string Property_Username = "Username";
    const std::string Property_Password = "Password";
    const std::string Property_NewPassword = "NewPassword";

public:
    LoginJsonTalker(const std::string& dbName);
    ~LoginJsonTalker();

    std::string parse_command(const std::string& input);

private:
    std::string process_check_match(const nlohmann::json& input);
    std::string process_change_username(const nlohmann::json& input);
    std::string process_change_password(const nlohmann::json& input);

    Sqlite3 theDb;
};

#endif