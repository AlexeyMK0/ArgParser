#include "ArgParser.h"

int ConvertToInt(const std::string& val) {
    int ret = 0;
    int k = 1;
    int ind = 0;
    if (val[0] == '-') {
        k = -1;
        ++ind;
    }
    while (ind < val.size()) {
        if (!isdigit(val[ind])) {
            // handle an error
        }
        ret *= 10;
        ret += val[ind] - '0';
        ++ind;
    }
    return ret;
}

namespace ArgumentParser {
    // BoolArg //
    ArgParser::BoolArg::BoolArg(const std::string& description, const char flag) :
        Node(description, flag) {}

    ArgParser::BoolArg::~BoolArg() {
        if (!stores_value_) return;
        if (stored_value_ != nullptr) {
            delete stored_value_;
        }
    }

    void ArgParser::BoolArg::Reset() {
        Node::Reset();
        CreateValuesIfNeed();
    }

    void ArgParser::BoolArg::AddValue(const std::string& val) {
        CreateValuesIfNeed();
        *stored_value_ = true;
        is_used_ = true;
    }

    bool ArgParser::BoolArg::IsOk() const {
        return true;
    }

    std::string ArgParser::BoolArg::GetRequirements(std::string sep) const {
        return Node::GetRequirements(sep);
    }


    ArgParser::BoolArg& ArgParser::BoolArg::Default(bool val) {
        has_default_ = true;
        default_val_ = val;
        return *this;
    }

    ArgParser::BoolArg& ArgParser::BoolArg::StoreValue(bool& storage) {
        if (stored_value_ != nullptr && !stores_value_) delete stored_value_;
        stored_value_ = &storage;
        stores_value_ = true;
        return *this;
    }


    bool ArgParser::BoolArg::GetValue() const {
        return *stored_value_;
    }

    void ArgParser::BoolArg::CreateValuesIfNeed() {
        if (stored_value_ == nullptr) {
            stored_value_ = new bool(default_val_);
        }
    }


    // HelpArg //
    ArgParser::HelpArg::HelpArg(const std::string& description, const char flag) :
        Node(description, flag) {}

    void ArgParser::HelpArg::Reset() {
        Node::Reset();
    }

    void ArgParser::HelpArg::AddValue(const std::string& val) {
        CreateValuesIfNeed();
        is_used_ = true;
    }

    bool ArgParser::HelpArg::IsOk() const {
        return true;
    }


    // PositionalNode //
    ArgParser::PositionalNode& ArgParser::PositionalNode::Positional() {
        is_positional_ = true;
        return *this; 
    }

    ArgParser::PositionalNode& ArgParser::PositionalNode::MultiValue(int min_size) {
        is_multivalue_ = true;
        min_size_ = min_size;
        return *this;
    }

    std::string ArgParser::PositionalNode::GetRequirements(std::string sep) const {
        std::string ret = Node::GetRequirements(sep);
        if (is_multivalue_) {
            AddSepIfNotNull(ret, sep);
            ret += "repeated";
        }
        if (is_positional_) {
            AddSepIfNotNull(ret, sep);
            ret += "positional";
        }
        if (min_size_ != kMinSizeDefault) {
            AddSepIfNotNull(ret, sep);
            ret += "min args = " + std::to_string(min_size_);
        }
        return ret;
    }
    
    // IntArg //
    ArgParser::IntArg::IntArg(const std::string& description, const char flag) :
        PositionalNode(description, flag) {}

    ArgParser::IntArg::~IntArg() {
        if (stores_value_) return;
        if (stored_value_ != nullptr) {
            delete stored_value_;
        }
        if (values_ != nullptr) {
            delete values_;
        }
    }

    void ArgParser::IntArg::Reset() {
        Node::Reset();
        CreateValuesIfNeed();
        if (IsMultiValue()) {
            values_->clear();
        } else if (has_default_) {
            *stored_value_ = default_val_;
        }
    }

    void ArgParser::IntArg::AddValue(const std::string& val) {
        CreateValuesIfNeed();
        is_used_ = true;
        int nval = ConvertToInt(val);
        if (IsMultiValue()) {
            values_->push_back(nval);
        } else {
            *stored_value_ = nval;
        }
    }

    bool ArgParser::IntArg::IsOk() const {
        if (has_default_) return true;
        if (IsMultiValue()) {
            return values_->size() >= min_size_; 
        } else {
            return is_used_;
        }
    }

    std::string ArgParser::IntArg::GetRequirements(std::string sep) const {
        std::string ret = PositionalNode::GetRequirements(sep);
        if (has_default_) {
            AddSepIfNotNull(ret, sep);
            ret += "default = " + std::to_string(default_val_);
        }
        return ret;
    }

    std::string ArgParser::IntArg::GetLongArg(const std::string& name) const {
        return Node::GetLongArg(name) + "=<int>";
    }

    ArgParser::IntArg& ArgParser::IntArg::Positional() {
        PositionalNode::Positional();
        return *this;
    }

    ArgParser::IntArg& ArgParser::IntArg::MultiValue(int min_size) {
        PositionalNode::MultiValue(min_size);
        if (values_ == nullptr) {
            values_ = new std::vector<int>();
        }
        return *this;
    }

    ArgParser::IntArg& ArgParser::IntArg::StoreValue(int& storage) {
        if (stored_value_ != nullptr && !stores_value_) delete stored_value_;
        stored_value_ = &storage;
        stores_value_ = true;
        return *this;
    }

    ArgParser::IntArg& ArgParser::IntArg::StoreValues(std::vector<int>& storage) {
        if (values_ != nullptr) delete values_;
        values_ = &storage;
        stores_value_ = true;
        return *this;
    }

    ArgParser::IntArg& ArgParser::IntArg::Default(int val) { 
        has_default_ = true;
        default_val_ = val;
        if (stored_value_ == nullptr)
            stored_value_ = new int(val);
        *stored_value_ = default_val_;
        return *this;
    }

    int ArgParser::IntArg::GetIntValue(int ind) const {
        if (!is_used_ && !has_default_) {
            // handle an error
        }
        if (IsMultiValue()) {
            return values_->at(ind);
        } else {
            return *stored_value_;
        }
    }

    void ArgParser::IntArg::CreateValuesIfNeed() {
        if (IsMultiValue()) {
            if (values_ == nullptr) {
                values_ = new std::vector<int>(1, 0);
            }
        } else if (stored_value_ == nullptr) {
            stored_value_ = new int(default_val_);
        }
    }


    // String arg //
    ArgParser::StringArg::StringArg(const std::string& description, const char flag) :
        PositionalNode(description, flag) {}
    ArgParser::StringArg::~StringArg() {
        if (stores_value_) return;
        if (stored_value_ != nullptr) {
            delete stored_value_;
        }
        if (values_ != nullptr) {
            delete values_;
        }
    }

    void ArgParser::StringArg::Reset() {
        Node::Reset();
        CreateValuesIfNeed();
        if (IsMultiValue()) {
            values_->clear();
        } else if (has_default_) {
            *stored_value_ = default_val_;
        }
    }

    void ArgParser::StringArg::AddValue(const std::string& val) {
        CreateValuesIfNeed();
        is_used_ = true;
        if (IsMultiValue()) {
            values_->push_back(val);
        } else {
            *stored_value_ = val;
        }
    }

    bool ArgParser::StringArg::IsOk() const {
        if (has_default_) return true;
        if (IsMultiValue()) {
            return values_->size() >= min_size_; 
        } else {
            return is_used_;
        }
    }

    std::string ArgParser::StringArg::GetRequirements(std::string sep) const {
        std::string ret = PositionalNode::GetRequirements(sep);
        if (has_default_) {
            AddSepIfNotNull(ret, sep);
            ret += "default = " + default_val_;
        }
        return ret;
    }

    std::string ArgParser::StringArg::GetLongArg(const std::string& name) const {
        return Node::GetLongArg(name) + "=<string>"; 
    }

    ArgParser::StringArg& ArgParser::StringArg::Positional() {
        PositionalNode::Positional();
        return *this;
    }

    ArgParser::StringArg& ArgParser::StringArg::MultiValue(int min_size) {
        PositionalNode::MultiValue(min_size);
        if (values_ == nullptr) {
            values_ = new std::vector<std::string>();
        }
        return *this;
    }

    ArgParser::StringArg& ArgParser::StringArg::StoreValue(std::string& storage) {
        if (stored_value_ != nullptr && !stores_value_) delete stored_value_;
        stored_value_ = &storage;
        stores_value_ = true;
        return *this;
    }

    ArgParser::StringArg& ArgParser::StringArg::StoreValues(std::vector<std::string>& storage) {
        if (values_ != nullptr) delete values_;
        values_ = &storage;
        stores_value_ = true;
        return *this;
    }

    ArgParser::StringArg& ArgParser::StringArg::Default(const std::string& val) {
        has_default_ = true;
        default_val_ = val;
        if (stored_value_ == nullptr)
            stored_value_ = new std::string(val);
        *stored_value_ = default_val_;
        return *this;
    }

    std::string ArgParser::StringArg::GetStringValue(int ind) const {
        if (!is_used_ && !has_default_) {
            // handle an error
        }
        if (IsMultiValue()) {
            return values_->at(ind);
        } else {
            return *stored_value_;
        }
    }

    void ArgParser::StringArg::CreateValuesIfNeed() {
        if (IsMultiValue()) {
            if (values_ == nullptr) {
                values_ = new std::vector<std::string>(1, kNullString);
            }
        } else if (stored_value_ == nullptr) {
            stored_value_ = new std::string(default_val_);
        }
    }
}