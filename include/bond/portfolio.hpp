#pragma once
#include "bond.hpp"
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

class Portfolio {
public:
    Portfolio() = default;
    explicit Portfolio(std::string name);

    void add_bond(const BondPtr& bond);
    void remove_bond(const std::string& bond_id);

    double calculate_total_value() const;
    double calculate_average_yield() const;
    std::vector<BondPtr> get_bonds_by_rating(const std::string& rating) const;
    std::vector<BondPtr> get_all_bonds() const;
    nlohmann::json to_json() const;

    const std::string& get_name() const { return name_; }

private:
    std::string name_;
    std::unordered_map<std::string, BondPtr> bonds_;
};