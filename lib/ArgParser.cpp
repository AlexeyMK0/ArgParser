#include "ArgParser.h"

#include <stdexcept>

namespace ArgumentParser {

const std::string ArgParser::kNullString = "";
const std::string ArgParser::kNoneParamName = kNullString;
const std::string ArgParser::kDefaultHelpDescription = "Display this help and exit";

ArgParser::ArgParser(const std::string& name) {
    name_ = name;
    name_to_argument_node_ = 
        std::unordered_map<std::string, std::unique_ptr<Node>>();
    flag_to_name_ = 
        std::vector<std::string>(kMaxFlagValue, kNoneParamName);
}


void ArgParser::AddHelp(const char flag, const std::string param_name, 
    const std::string& description) 
{
    CheckAddNewArg(flag, param_name);
    AddArgument(flag, param_name, 
        new HelpArg(kDefaultHelpDescription, flag));
    help_node_param_ = param_name;
    program_description_ = description;
}

bool ArgParser::Help() {
    if (help_node_param_ == kNoneParamName) return false;
    return GetHelpArg().IsUsed();
}

std::string ArgParser::HelpDescription() {
    std::string ret = "Parser name: " + name_ + "\n" +
        program_description_ + "\n";
    for (const auto& [param, ptr] : name_to_argument_node_) {
        if (param == help_node_param_) continue;
        ret += GetArgInfo(*ptr, param) + "\n";
    }
    ret += GetArgInfo(GetHelpArg(), help_node_param_);
    return ret;
}

std::string ArgParser::GetStringValue(std::string param, int ind) {
    AssertType(ArgType::kStringArg, param);
    StringArg& arg = GetStringArg(param);
    return arg.GetStringValue(ind);
}

bool ArgParser::GetFlag(std::string param) {
    AssertType(ArgType::kBoolArg, param);
    BoolArg& arg = GetBoolArg(param);
    return arg.GetValue();
}

int ArgParser::GetIntValue(std::string param, int ind) {
    AssertType(ArgType::kIntArg, param);
    IntArg& arg = GetIntArg(param);
    return arg.GetIntValue(ind);
}

ArgParser::IntArg& ArgParser::AddIntArgument(const char flag, 
    const std::string& param_name, const std::string& description) 
{
    CheckAddNewArg(flag, param_name);
    AddArgument(flag, param_name, new IntArg(description, flag));
    return GetIntArg(param_name);
}
ArgParser::IntArg& ArgParser::AddIntArgument(const std::string& param_name, 
    const std::string& description) 
{
    return AddIntArgument(kNoneFlag, param_name, description);
}

ArgParser::StringArg& ArgParser::AddStringArgument(const char flag,
    const std::string& param_name, const std::string& description)
{
    CheckAddNewArg(flag, param_name);
    AddArgument(flag, param_name, new StringArg(description, flag));
    return GetStringArg(param_name);
}
ArgParser::StringArg& ArgParser::AddStringArgument(const std::string& param_name,
    const std::string& description)
{
    return AddStringArgument(kNoneFlag, param_name, description);
}

ArgParser::BoolArg& ArgParser::AddFlag(const char flag, 
    const std::string& param_name, const std::string& description)
{
    CheckAddNewArg(flag, param_name);
    AddArgument(flag, param_name, new BoolArg(description, flag));
    return GetBoolArg(param_name);
}

ArgParser::BoolArg& ArgParser::AddFlag(const std::string& param_name, 
    const std::string& description)
{
    return AddFlag(kNoneFlag, param_name, description);
}

std::string ArgParser::GetParamByFlag(const char flag) const {
    return flag_to_name_[flag];
}



void ArgParser::AssertType(ArgType type, const std::string &param_name) const {
    if (!CheckType(type, param_name)) {
        throw std::runtime_error("Miss type error. Param name: " + param_name);
    }
}

bool ArgParser::CheckType(ArgType type, const std::string& param_name) const {
    auto node = name_to_argument_node_.find(param_name);
    if (node == name_to_argument_node_.end()) {
        return false;
    }
    return CheckType(type, node->second);
}

bool ArgParser::CheckType(ArgType type, const std::unique_ptr<Node>& node) const {
    return node->GetType() == type; 
}

bool ArgParser::CheckPositional(const std::string& param_name) const {
    if (!CheckType(ArgType::kIntArg, param_name) &&
        !CheckType(ArgType::kStringArg, param_name))
    {
        return false;
    }
    return static_cast<PositionalNode&>(*name_to_argument_node_.at(param_name)).IsPositional();
}

bool ArgParser::CheckAddNewArg(const char flag, 
    const std::string& param_name) const
{
    if (!CheckType(ArgType::kNone, param_name)) {
        return false;
    }
    if (flag_to_name_[flag] != kNoneParamName) {
        return false;
    }
    return true;
}

bool ArgParser::CheckArgsAreOk() {
    for (const auto& [param, ptr] : name_to_argument_node_) {
        if (param == help_node_param_) {
            continue;
        }
        if (!ptr->IsOk()) {
            return false;
        }
    }
    return true;
}

bool ArgParser::ValidateParam(const std::string& param) const {
    if (param == kNoneParamName) return false;
    return !CheckType(ArgType::kNone, param);
}

bool ArgParser::ValidateFlag(const char flag) const {
    if (flag == kNoneFlag) return false;
    return flag_to_name_[flag] != kNoneParamName;
}

void ArgParser::AddArgument(const char flag, 
    const std::string& param_name, Node* arg_ptr)
{
    Update();
    name_to_argument_node_[param_name] = 
        std::unique_ptr<Node>(arg_ptr);
    if (flag != kNoneFlag) {
        flag_to_name_[flag] = param_name;
    }
    need_update_ = true;
    last_added_param_ = param_name;
}

void ArgParser::ArgCalled(const std::string& param) {
    GetArg(param).ArgCalled();
}

void ArgParser::SetPositional(const std::string& param) {
    if (positional_param_ != kNoneParamName) {
        throw std::runtime_error("Positional argument could be only one");        
    }
    positional_param_ = param;
}

bool ArgParser::AddToPostional(const std::string& val) {
    Update();
    if (positional_param_ == kNoneParamName) {
        return false;
    }
    Node& node = GetArg(positional_param_);
    return node.AddValue(val);
}

void ArgParser::Update() {
    if (!need_update_) return;
    if (last_added_param_ != kNoneParamName && 
        CheckPositional(last_added_param_))
    {
        SetPositional(last_added_param_);
        last_added_param_ = kNoneParamName;
    }
    need_update_ = false;
}

void ArgParser::Reset() {
    Update();
    for (auto& [param, ptr] : name_to_argument_node_) {
        ptr->Reset();
    }
    good_parse_ = true;
}

ArgParser::Node& ArgParser::GetArg(const std::string& param) {
    return *name_to_argument_node_[param];
}

ArgParser::HelpArg& ArgParser::GetHelpArg() {
    if (!CheckType(ArgType::kHelp, help_node_param_)) {
        throw std::runtime_error("Help argument is missed or is duplicated by other argument");
    }
    return static_cast<HelpArg&>(GetArg(help_node_param_));
}

ArgParser::IntArg& ArgParser::GetIntArg(const std::string& param) {
    if (!CheckType(ArgType::kIntArg, param)) {
        throw std::runtime_error(param + " is not int arg");
    }
    return static_cast<IntArg&>(GetArg(param));
}

ArgParser::StringArg& ArgParser::GetStringArg(const std::string& param) {
    if (!CheckType(ArgType::kStringArg, param)) {
        throw std::runtime_error(param + " is not string arg");
    }
    return static_cast<StringArg&>(GetArg(param));
}

ArgParser::BoolArg& ArgParser::GetBoolArg(const std::string& param) {
    if (!CheckType(ArgType::kBoolArg, param)) {
        throw std::runtime_error(param + " is not bool arg");
    }
    return static_cast<BoolArg&>(GetArg(param));
}

std::string ArgParser::GetArgInfo(const Node& val, const std::string& name) {
    std::string ret = kNullString;
    ret = val.GetFlag() + "  " + 
        val.GetLongArg(name) + "  " +
        val.GetDescription() + "  ";
    std::string reqs = val.GetRequirements();
    if (reqs != kNullString) {
        ret += "[" + reqs + "]";
    }
    return ret;
}

} // namespace ArgumentParser