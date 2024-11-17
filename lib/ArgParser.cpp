#include "ArgParser.h"

#include <stdexcept>

namespace ArgumentParser {

const std::string ArgParser::kNullString = "";
const std::string ArgParser::kNoneParamName = kNullString;
const std::string ArgParser::kDefaultHelpDescription = "Display this help and exit";
// const std::unique_ptr<ArgParser::Node> ArgParser::Node::kNullNodePtr {nullptr};

ArgParser::ArgParser(const std::string& name) {
    name_ = name;
    name_to_argument_node_ = 
        std::unordered_map<std::string, std::unique_ptr<Node>>();
    flag_to_name_ = 
        std::vector<std::string>(kMaxFlagValue, kNoneParamName);
}

// ArgParser::~ArgParser();

// bool ArgParser::Parse(const int argc, char** argv);
bool ArgParser::Parse(const std::vector<std::string>& args) {
    int cnt = 0;
    for (const auto& val : args) {
        if (cnt & 1) {
            GetArg("goal").AddValue(val);
        } else {
            // GetArg("bebra").AddValue(val);
            AddToPostional(val);
        }
        ++cnt;
    }
    GetArg("help").AddValue("");
    return CheckArgsAreOk();
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
    // HelpArg& helpNode = static_cast<HelpArg&>(GetArg(help_node_param_));
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
    if (!CheckType(ArgType::kStringArg, param)) {
        // handle an error
    }
    StringArg& arg = GetStringArg(param);
    return arg.GetStringValue(ind);
}

bool ArgParser::GetFlag(std::string param) {
    if (!CheckType(ArgType::kBoolArg, param)) {
        // handle an error
    }
    BoolArg& arg = GetBoolArg(param);
    return arg.GetValue();
}

int ArgParser::GetIntValue(std::string param, int ind) {
    if (!CheckType(ArgType::kIntArg, param)) {
        // handle an error
    }
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


// std::unique_ptr<ArgParser::Node>& ArgParser::GetPtr(const std::string& param) {
//     auto node = name_to_argument_node_.find(param);
//     if (node == name_to_argument_node_.end()) {
//         // handle an error
//     }
//     return node->second;
// }

// void ArgParser::AssertNotNone(const std::string& param_name) const {
    
// }

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
    return CheckType(ArgType::kIntArg, param_name) || 
        CheckType(ArgType::kStringArg, param_name);
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
        if (!ptr->IsOk()) return false;
    }
    return true;
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

void ArgParser::SetPositional(const std::string& param) {
    if (positional_param_ != kNoneParamName) {
        // handle an error
    }
    positional_param_ = param;
}

void ArgParser::AddToPostional(const std::string& val) {
    Update();
    if (positional_param_ == kNoneParamName) {
        // handle an error
    }
    Node& node = GetArg(positional_param_);
    node.AddValue(val);
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

ArgParser::Node& ArgParser::GetArg(const std::string& param) {
    if (CheckType(ArgType::kNone, param)) {
        // handle an error
    }
    return *name_to_argument_node_[param];
}

ArgParser::HelpArg& ArgParser::GetHelpArg() {
    if (!CheckType(ArgType::kHelp, help_node_param_)) {
        // handle an error
    }
    return static_cast<HelpArg&>(*name_to_argument_node_[help_node_param_]);
}

ArgParser::IntArg& ArgParser::GetIntArg(const std::string& param) {
    if (!CheckType(ArgType::kIntArg, param)) {
        throw std::runtime_error(param + " is not int arg");
    }
    return static_cast<IntArg&>(*name_to_argument_node_[param]);
}

ArgParser::StringArg& ArgParser::GetStringArg(const std::string& param) {
    if (!CheckType(ArgType::kStringArg, param)) {
        throw std::runtime_error(param + " is not string arg");
    }
    return static_cast<StringArg&>(*name_to_argument_node_[param]);
}

ArgParser::BoolArg& ArgParser::GetBoolArg(const std::string& param) {
    if (!CheckType(ArgType::kBoolArg, param)) {
        throw std::runtime_error(param + " is not bool arg");
    }
    return static_cast<BoolArg&>(*name_to_argument_node_[param]);
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