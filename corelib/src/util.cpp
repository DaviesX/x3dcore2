#include "util.h"

e8util::flex_config::flex_config() {}

template <typename T>
static std::map<std::string, T> val_diff(std::map<std::string, T> const &a,
                                         std::map<std::string, T> const &b) {
    std::map<std::string, T> diff_a_from_b;
    for (auto const &pair_a : a) {
        auto it_b = b.find(pair_a.first);
        if (it_b == b.end()) {
            diff_a_from_b.insert(pair_a);
        } else {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
            if (it_b->second != pair_a.second) {
#pragma GCC diagnostic pop
                diff_a_from_b.insert(pair_a);
            }
        }
    }
    return diff_a_from_b;
}

e8util::flex_config e8util::flex_config::operator-(flex_config const &other) const {
    flex_config diff_this_from_other;
    diff_this_from_other.bool_val = val_diff(bool_val, other.bool_val);
    diff_this_from_other.int_val = val_diff(int_val, other.int_val);
    diff_this_from_other.float_val = val_diff(float_val, other.float_val);
    diff_this_from_other.str_val = val_diff(str_val, other.str_val);
    diff_this_from_other.bool_val = val_diff(bool_val, other.bool_val);
    diff_this_from_other.enum_sel = val_diff(enum_sel, other.enum_sel);
    diff_this_from_other.enum_vals = val_diff(enum_vals, other.enum_vals);
    for (auto const &this_enum_vals_configs : enum_val_configs) {
        auto it_other = other.enum_val_configs.find(this_enum_vals_configs.first);
        if (it_other == other.enum_val_configs.end()) {
            diff_this_from_other.enum_val_configs.insert(this_enum_vals_configs);
        } else {
            flex_config const &other_config = this_enum_vals_configs.second - it_other->second;
            diff_this_from_other.enum_val_configs.insert(
                std::make_pair(this_enum_vals_configs.first, other_config));
        }
    }
    return diff_this_from_other;
}

e8util::not_implemented_exception::not_implemented_exception(std::string const &func_name)
    : std::logic_error(func_name + "() has not been implemented.") {}

e8util::not_implemented_exception::not_implemented_exception()
    : std::logic_error("Function has not been implemented.") {}

e8util::entity_not_found_exception::entity_not_found_exception()
    : std::out_of_range("Entity cannot be found.") {}

e8util::entity_not_found_exception::entity_not_found_exception(std::string const &entity,
                                                               std::string const &id)
    : std::out_of_range("Entity " + entity + " cannot be found with the identifier " + id) {}
