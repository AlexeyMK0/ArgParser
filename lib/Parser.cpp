#include "ArgParser.h"


namespace ArgumentParser {

ArgParser::ParseData::ParseData(const std::string& cur_param_name, const bool cur_param_got_arg) :
    cur_param_name(cur_param_name), cur_param_got_arg(cur_param_got_arg)
{
    cur_parse_arg = kNullString;
}

bool ArgParser::Parse(const int argc, char** argv) {
    std::vector<std::string> args;
    args.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    return Parse(args);
}

bool ArgParser::Parse(const std::vector<std::string>& args) {
    good_parse_ = true;
    int argc = args.size();
    ParseData parse_data;
    parse_data.next_ind = 1;
    // args[0] stands for program name
    while (true) {
        if (parse_data.cur_parse_arg == kNullString) {
            if (parse_data.next_ind >= argc){
                break;
            }
            parse_data.cur_parse_arg = args[parse_data.next_ind++];
            parse_data.cur_type = GetParseArgType(parse_data.cur_parse_arg);
        }
        switch (parse_data.cur_type)
        {
        case ParseArgType::kValue:
            good_parse_ &= ProcessValue(parse_data);
            break;
        case ParseArgType::kFlag:
            good_parse_ &= ProcessFlag(parse_data);
            break;
        case ParseArgType::kArgument:
            good_parse_ &= ProcessArgument(parse_data);
            break;
        case ParseArgType::kEmpty:
            continue;
        default:
            break;
        }
    }
    good_parse_ &= CheckArgsAreOk();
    return good_parse_;
}

bool ArgParser::ProcessValue(ParseData& parse_data) {
    bool is_good = true;
    if (parse_data.cur_param_name == kNoneParamName) {
        is_good &= AddToPostional(parse_data.cur_parse_arg);
    } else if (GetArg(parse_data.cur_param_name).TakesArgument() &&
        (GetArg(parse_data.cur_param_name).IsMultiValue() || !parse_data.cur_param_got_arg))
    {
        is_good &= GetArg(parse_data.cur_param_name).AddValue(parse_data.cur_param_name);
    } else {
        is_good &= AddToPostional(parse_data.cur_parse_arg);
    }
    parse_data.cur_parse_arg = kNullString;
    parse_data.cur_type = ParseArgType::kEmpty;
    parse_data.cur_param_got_arg = true;
    return is_good;
}

bool ArgParser::ProcessFlag(ParseData& parse_data) {
    parse_data.cur_param_got_arg = false;
    for (int i = 1; i < parse_data.cur_parse_arg.size(); ++i) {
        char flag = parse_data.cur_parse_arg[i];
        parse_data.cur_param_name = GetParamByFlag(flag);
        ArgCalled(parse_data.cur_param_name);
    }
    parse_data.cur_type = ParseArgType::kEmpty;
    return true;
}

bool ArgParser::ProcessArgument(ParseData& parse_data) {
    parse_data.cur_param_got_arg = false;
    int arg_size = parse_data.cur_parse_arg.size();
    std::string param = GetParamByLongArg(parse_data.cur_parse_arg);
    ArgCalled(param);
    parse_data.cur_param_name = param;
    if (param.size() == arg_size - 2) {
        parse_data.cur_parse_arg = kNullString;
        parse_data.cur_type = ParseArgType::kEmpty;
    } else {
        parse_data.cur_parse_arg = parse_data.cur_parse_arg.substr(2 + param.size());
        parse_data.cur_type = ParseArgType::kValue;
    }
    return true;
}

ArgParser::ParseArgType ArgParser::GetParseArgType(const std::string& arg) const {
    if (arg.empty()) {
        return ParseArgType::kEmpty;
    }
    if (arg == "-" || arg == "--" || arg[0] != '-') {
        return ParseArgType::kValue;
    }
    if (arg[1] != '-') {
        for (int i = 1; i < arg.size(); ++i) {
            if (!ValidateFlag(arg[i])) {
                return ParseArgType::kValue;
            }
        }
        return ParseArgType::kFlag;
    }
    // arg[0, 1] == "--"
    std::string potential_argument = GetParamByLongArg(arg);
    if (ValidateParam(potential_argument)) {
        return ParseArgType::kArgument;
    }
    return ParseArgType::kValue;
}

std::string ArgParser::GetParamByLongArg(const std::string& long_arg) const {
    if (long_arg.size() < 2) {
        return kNullString;
    }
    size_t end_of_argument = long_arg.find('=');
    end_of_argument = end_of_argument == std::string::npos ? 
        long_arg.size() : end_of_argument;
    std::string param = long_arg.substr(2, end_of_argument - 2);
    return param;
}

}
