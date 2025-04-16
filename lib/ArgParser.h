#pragma once

#include <numeric>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include <iostream>

namespace ArgumentParser {

class ArgParser {
    const static int kMaxFlagValue = 256;
    const static char kNoneFlag = '\0';
    const static int kMinSizeDefault = 1;
    const static std::string kNullString;
    const static std::string kNoneParamName;
    const static std::string kDefaultHelpDescription;
private:
    enum class ArgType {
        kIntArg = 0,
        kBoolArg,
        kStringArg,
        kHelp,
        kNone
    };

    enum class ParseArgType {
        kValue = 0,
        kFlag,
        kArgument,
        kEmpty
    };

    struct ParseData {
        std::string cur_param_name;
        bool cur_param_got_arg;
        std::string cur_parse_arg;
        ParseArgType cur_type;
        int next_ind;
        ParseData(const std::string& cur_param_name = kNoneParamName, 
            const bool cur_param_got_arg = false);
    };

    class Node {
     protected:
        Node(const std::string& description, const char flag);
     public:
        virtual void Reset() { is_used_ = false; }
        virtual ArgType GetType() const { return ArgType::kNone; }
        virtual bool AddValue(const std::string& val)
            {return true;}
        virtual void ArgCalled() {}
        virtual bool IsOk() const { return true; }
        virtual bool TakesArgument() const;
        virtual std::string GetDescription() const { return description_; }
        virtual std::string GetFlag() const;
        virtual std::string GetLongArg(const std::string& name_) const
            { return "--" + name_; }
        virtual std::string GetRequirements(std::string sep = ", ") const 
            { return kNullString; }
        bool IsUsed() const { return is_used_; }
        bool IsMultiValue() const { return is_multivalue_; }
     protected:
        void AddSepIfNotNull(std::string& val, const std::string& sep) const;
        virtual void CreateValuesIfNeed() {}
        bool is_used_ = false;
        bool has_default_ = false;
        bool stores_value_ = false;
        bool is_multivalue_ = false;
        std::string description_ = kNullString;
        char flag_ = kNoneFlag;
    };

    class BoolArg : public Node {
     public:
        BoolArg(const std::string& description, const char flag);
        ~BoolArg();
        virtual void Reset() override;
        virtual ArgType GetType() const override { return ArgType::kBoolArg; }
        virtual bool AddValue(const std::string& val) override;
        virtual void ArgCalled() override;
        virtual bool IsOk() const override;
        virtual std::string GetRequirements(std::string sep = ", ") const override;
        BoolArg& Default(bool val);
        BoolArg& StoreValue(bool& storage);
        bool GetValue() const;
     protected:
        virtual void CreateValuesIfNeed() override;
     private:
        bool default_val_ = false;
        bool* stored_value_ = nullptr;
    };

    class HelpArg : public Node {
     public:
        HelpArg(const std::string& description, const char flag);
        virtual void Reset() override;
        virtual ArgType GetType() const override { return ArgType::kHelp; }
        virtual bool AddValue(const std::string& val) override;
        virtual void ArgCalled() override;
        virtual bool IsOk() const override;
    protected:
        virtual void CreateValuesIfNeed() override {}
    };

    class PositionalNode : public Node {
     public:
        virtual PositionalNode& Positional();
        virtual PositionalNode& MultiValue(int min_size = kMinSizeDefault);
        bool IsPositional() const { return is_positional_; }
        virtual std::string GetRequirements(std::string sep = ", ") const override;
     protected:
        PositionalNode(const std::string& description, const char flag) 
        : Node(description, flag) {}
        bool is_positional_ = false;
        int min_size_ = kMinSizeDefault;
    };

    class IntArg : public PositionalNode {
     public:
        IntArg(const std::string& description, const char flag);
        ~IntArg();
        virtual void Reset() override;
        virtual ArgType GetType() const override { return ArgType::kIntArg; }
        virtual bool AddValue(const std::string& val) override;
        virtual bool IsOk() const override;
        virtual bool TakesArgument() const override { return true; }
        virtual std::string GetRequirements(std::string sep = ", ") const override;
        virtual std::string GetLongArg(const std::string& name) const override; 
        virtual IntArg& Positional() override;
        virtual IntArg& MultiValue(int min_size = kMinSizeDefault) override;
        virtual IntArg& StoreValue(int& storage);
        IntArg& StoreValues(std::vector<int>& storage);
        IntArg& Default(int val);
        int GetIntValue(int ind = 0) const;
     protected:
        virtual void CreateValuesIfNeed() override;
     private:
        int default_val_ = 0;
        int* stored_value_ = nullptr;
        std::vector<int>* values_ = nullptr;
    };


    class StringArg : public PositionalNode {
     public:
        StringArg(const std::string& description, const char flag);
        ~StringArg();
        virtual void Reset() override;
        virtual ArgType GetType() const override { return ArgType::kStringArg; }
        virtual bool AddValue(const std::string& val) override;
        virtual bool IsOk() const override;
        virtual bool TakesArgument() const override { return true; }
        virtual std::string GetRequirements(std::string sep = ", ") const override;
        virtual std::string GetLongArg(const std::string& name) const override; 
        virtual StringArg& Positional() override;
        virtual StringArg& MultiValue(int min_size = kMinSizeDefault) override;
        StringArg& StoreValue(std::string& storage);
        StringArg& StoreValues(std::vector<std::string>& storage);
        StringArg& Default(const std::string& val);
        std::string GetStringValue(int ind = 0) const;
    protected:
        virtual void CreateValuesIfNeed() override;
     private:
        std::string default_val_ = kNullString;
        std::string* stored_value_ = nullptr;
        std::vector<std::string>* values_ = nullptr;
    };

public:
    ArgParser(const std::string& name);
    bool Parse(const int argc, char** argv);
    bool Parse(const std::vector<std::string>& args);
    bool ProcessValue(ParseData& parse_data);
    bool ProcessFlag(ParseData& parse_data);
    bool ProcessArgument(ParseData& parse_data);
    void AddHelp(const char flag, const std::string param_name, const std::string& description);
    bool Help();
    std::string HelpDescription();

    std::string GetStringValue(std::string param, int ind = 0);
    bool GetFlag(std::string param);
    int GetIntValue(std::string param, int ind = 0);

    IntArg& AddIntArgument(const char flag, const std::string& param_name, 
        const std::string& description = "");
    IntArg& AddIntArgument(const std::string& param_name, const std::string& description = "");
    
    StringArg& AddStringArgument(const char flag, const std::string& param_name, 
        const std::string& description = "");
    StringArg& AddStringArgument(const std::string& param_name, const std::string& description = "");

    BoolArg& AddFlag(const char flag, const std::string& param_name, 
        const std::string& description = "");
    BoolArg& AddFlag(const std::string& param_name, const std::string& description = "");

    std::string GetParamByFlag(const char flag) const;

private:
    void AssertType(ArgType type, const std::string &param_name) const;
    bool CheckType(ArgType type, const std::string &param_name) const;
    bool CheckType(ArgType type, const std::unique_ptr<Node>& node) const;
    bool CheckPositional(const std::string& param_name) const;
    bool CheckAddNewArg(const char flag, const std::string& param_name) const;
    bool CheckArgsAreOk();
    bool ValidateParam(const std::string& param) const;
    bool ValidateFlag(const char flag) const;
    void AddArgument(const char flag, const std::string& param_name, 
        Node* arg_ptr);
    void ArgCalled(const std::string& param);
    void SetPositional(const std::string& param);
    bool AddToPostional(const std::string& val);
    void Update();
    void Reset();
    // std::unique_ptr<Node> CreateNode(ArgType type);
    Node& GetArg(const std::string& param);
    HelpArg& GetHelpArg();
    IntArg& GetIntArg(const std::string& param);
    StringArg& GetStringArg(const std::string& param);
    BoolArg& GetBoolArg(const std::string& param);
    std::string GetArgInfo(const Node& val, const std::string& name);

    ParseArgType GetParseArgType(const std::string& arg) const;
    std::string GetParamByLongArg(const std::string& long_arg) const;

    std::string positional_param_ = kNoneParamName;
    std::string last_added_param_ = kNoneParamName;
    std::string help_node_param_ = kNoneParamName;
    std::string program_description_ = kNullString;
    std::string name_;
    bool need_update_ = false;
    bool good_parse_ = true;

    std::unordered_map<std::string, std::unique_ptr<Node>> name_to_argument_node_;
    std::vector<std::string> flag_to_name_;
};

} // namespace ArgumentParser