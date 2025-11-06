#pragma once
#include "portfolio.hpp"
#include "../db/database.hpp"
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

class BondService {
public:
    BondService(std::shared_ptr<IDatabase> primary_db,
                std::shared_ptr<IDatabase> analytics_db);

    bool add_bond(const Bond& bond);
    std::vector<Bond> get_high_yield_bonds(double threshold);
    void create_portfolio(const std::string& name);
    std::vector<Bond> get_portfolio_bonds(const std::string& portfolio_name);
    double calculate_portfolio_value(const std::string& portfolio_name);

private:
    std::shared_ptr<IDatabase> primary_db_;
    std::shared_ptr<IDatabase> analytics_db_;
    std::unordered_map<std::string, Portfolio> portfolios_;

    void ensure_portfolio_exists(const std::string& name);
};