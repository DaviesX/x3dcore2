#ifndef UTIL_H
#define UTIL_H

#include <map>
#include <set>

namespace e8util {

class flex_config;

template<typename T>
struct config_val_visitor
{
    void operator()(T const & /* val */) {}
};

struct config_enum_visitor
{
    void operator()(std::string const & /* enum_sel */, flex_config const * /* enum_config */) {}
};

class flex_config
{
public:
    flex_config();
    flex_config operator-(flex_config const &other) const;
    template<typename ReadOp>
    void find_bool(std::string const &key, ReadOp read_op) const;
    template<typename ReadOp>
    void find_int(std::string const &key, ReadOp read_op) const;
    template<typename ReadOp>
    void find_float(std::string const &key, ReadOp read_op) const;
    template<typename ReadOp>
    void find_str(std::string const &key, ReadOp read_op) const;
    template<typename ReadOpEnum>
    void find_enum(std::string const &key, ReadOpEnum read_op) const;

    std::map<std::string, bool> bool_val;
    std::map<std::string, int> int_val;
    std::map<std::string, float> float_val;
    std::map<std::string, std::string> str_val;
    std::map<std::string, std::set<std::string>> enum_vals;
    std::map<std::string, flex_config> enum_val_configs;
    std::map<std::string, std::string> enum_sel;
};

template<typename ReadOp>
void flex_config::find_bool(std::string const &key, ReadOp read_op) const
{
    auto it = bool_val.find(key);
    if (it != bool_val.end()) {
        read_op(it->second);
    }
}

template<typename ReadOp>
void flex_config::find_int(std::string const &key, ReadOp read_op) const
{
    auto it = int_val.find(key);
    if (it != int_val.end()) {
        read_op(it->second);
    }
}

template<typename ReadOp>
void flex_config::find_float(std::string const &key, ReadOp read_op) const
{
    auto it = float_val.find(key);
    if (it != float_val.end()) {
        read_op(it->second);
    }
}

template<typename ReadOp>
void flex_config::find_str(std::string const &key, ReadOp read_op) const
{
    auto it = str_val.find(key);
    if (it != str_val.end()) {
        read_op(it->second);
    }
}

template<typename ReadOpEnum>
void flex_config::find_enum(std::string const &key, ReadOpEnum read_op) const
{
    auto it = enum_sel.find(key);
    if (it != enum_sel.end()) {
        auto config_it = enum_val_configs.find(key + "." + it->second);
        read_op(it->second, config_it != enum_val_configs.end() ? &config_it->second : nullptr);
    }
}

} // namespace e8util

#endif // UTIL_H
