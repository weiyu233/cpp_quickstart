#include "../../include/bond/bond_service.hpp"

BondService::BondService(std::shared_ptr<IDatabase> primary_db,
                         std::shared_ptr<IDatabase> analytics_db)
    : primary_db_(std::move(primary_db)), analytics_db_(std::move(analytics_db)) {}

bool BondService::add_bond(const Bond& bond) {
    if (!primary_db_->save_bond(bond)) return false;
    analytics_db_->save_bond(bond);
    ensure_portfolio_exists(bond.get_rating());
    portfolios_[bond.get_rating()].add_bond(std::make_shared<Bond>(bond));
    return true;
}

std::vector<Bond> BondService::get_high_yield_bonds(double threshold) {
    std::vector<Bond> result;
    for (const auto& [name, portfolio] : portfolios_) {
        for (const auto& bond : portfolio.get_all_bonds()) {
            if (bond->get_yield() > threshold) {
                result.push_back(*bond);
            }
        }
    }
    return result;
}

void BondService::create_portfolio(const std::string& name) {
    if (portfolios_.find(name) == portfolios_.end()) {
        portfolios_.emplace(name, Portfolio(name));
    }
}

std::vector<Bond> BondService::get_portfolio_bonds(const std::string& portfolio_name) {
    if (portfolios_.find(portfolio_name) == portfolios_.end()) return {};
    std::vector<Bond> result;
    for (const auto& bond_ptr : portfolios_[portfolio_name].get_all_bonds()) {
        result.push_back(*bond_ptr);
    }
    return result;
}

double BondService::calculate_portfolio_value(const std::string& portfolio_name) {
    if (portfolios_.find(portfolio_name) == portfolios_.end()) return 0.0;
    return portfolios_[portfolio_name].calculate_total_value();
}

void BondService::ensure_portfolio_exists(const std::string& name) {
    if (portfolios_.find(name) == portfolios_.end()) {
        portfolios_.emplace(name, Portfolio(name));
    }
}