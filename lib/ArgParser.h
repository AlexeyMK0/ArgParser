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
    enum ArgType {
        kIntArg = 0,
        kBoolArg,
        kStringArg,
        kHelp,
        kNone
    };

    class Node {
     public:
        const static std::unique_ptr<Node> kNullNodePtr; 
     protected:
        Node(const std::string& description, const char flag) {
            description_ = description;
            flag_ = flag;
        }
     public:
        virtual std::string GetFlag() const {
            std::string desc = "";
            if (flag_ != kNoneFlag) {
                desc.push_back('-');
                desc.push_back(flag_);
            } else {
                desc = "  ";
            }
            return desc;
        }
        virtual void Reset() { is_used_ = false; }
        virtual ArgType GetType() const { return ArgType::kNone; }
        virtual void AddValue(const std::string& val) 
            {std::cout << "This should never be called\n";}
        virtual bool IsOk() const { return true; }
        virtual std::string GetDescription() const { return description_; }
        virtual std::string GetLongArg(const std::string& name_) const
            { return "--" + name_; }
        virtual std::string GetFlag() {
            std::string ret = "  ";
            if (flag_ != kNoneFlag) {
                ret = "-";
                ret.push_back(flag_);
            }
            return ret;
        }
        virtual std::string GetRequirements(std::string sep = ", ") const 
            { return kNullString; }
        bool IsUsed() const { return is_used_; }
     protected:
        void AddSepIfNotNull(std::string& val, const std::string& sep) const {
            if (val != kNullString) {
                val += sep;
            }
        }
        virtual void CreateValuesIfNeed() {};
        // int max_size_ = std::numeric_limits<int>::max();
        bool is_used_ = false;
        bool has_default_ = false;
        bool stores_value_ = false;
        std::string description_ = kNullString;
        char flag_ = kNoneFlag;
    };

    class BoolArg : public Node {
     public:
        BoolArg(const std::string& description, const char flag);
        ~BoolArg();
        virtual void Reset() override;
        virtual ArgType GetType() const override { return ArgType::kBoolArg; }
        virtual void AddValue(const std::string& val) override;
        virtual bool IsOk() const override;
        virtual std::string GetRequirements(std::string sep = ", ") const override;
        BoolArg& Default(bool val);
        BoolArg& StoreValue(bool& storage);
        bool GetValue() const;
     protected:
        virtual void CreateValuesIfNeed() override;
     private:
        bool default_val_ = false;
        bool* stored_value_;
    };

    class HelpArg : public Node {
     public:
        HelpArg(const std::string& description, const char flag);
        virtual void Reset() override;
        virtual ArgType GetType() const override { return ArgType::kHelp; }
        virtual void AddValue(const std::string& val) override;
        virtual bool IsOk() const override;
    protected:
        virtual void CreateValuesIfNeed() override {}
    };

    class PositionalNode : public Node {
     public:
        virtual PositionalNode& Positional();
        virtual PositionalNode& MultiValue(int min_size = kMinSizeDefault);
        bool IsPositional() const { return is_positional_; }
        bool IsMultiValue() const { return is_multivalue_; }
        virtual std::string GetRequirements(std::string sep = ", ") const override;
     protected:
        PositionalNode(const std::string& description, const char flag) 
        : Node(description, flag) {}
        bool is_positional_ = false;
        bool is_multivalue_ = false;
        int min_size_ = kMinSizeDefault;
    };

    class IntArg : public PositionalNode {
     public:
        IntArg(const std::string& description, const char flag);
        ~IntArg();
        virtual void Reset() override;
        virtual ArgType GetType() const override { return ArgType::kIntArg; }
        virtual void AddValue(const std::string& val) override;
        virtual bool IsOk() const override;
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
        virtual void AddValue(const std::string& val) override;
        virtual bool IsOk() const override;
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
        std::string default_val_ = "";
        std::string* stored_value_ = nullptr;
        std::vector<std::string>* values_ = nullptr;
    };

public:
    ArgParser(const std::string& name);
    // ~ArgParser();
    // bool Parse(const int argc, char** argv);
    bool Parse(const std::vector<std::string>& args);

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
    
private:
    // std::unique_ptr<Node>& GetPtr(const std::string& param);
    bool CheckType(ArgType type, const std::string &param_name) const;
    bool CheckType(ArgType type, const std::unique_ptr<Node>& node) const;
    bool CheckPositional(const std::string& param_name) const;
    bool CheckAddNewArg(const char flag, const std::string& param_name) const;
    bool CheckArgsAreOk();
    void AddArgument(const char flag, const std::string& param_name, 
        Node* arg_ptr);
    void SetPositional(const std::string& param);
    void AddToPostional(const std::string& val);
    void Update();
    // std::unique_ptr<Node> CreateNode(ArgType type);
    Node& GetArg(const std::string& param);
    HelpArg& GetHelpArg();
    IntArg& GetIntArg(const std::string& param);
    StringArg& GetStringArg(const std::string& param);
    BoolArg& GetBoolArg(const std::string& param);
    std::string GetArgInfo(const Node& val, const std::string& name);

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