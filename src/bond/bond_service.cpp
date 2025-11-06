#include "../../include/bond/bond_service.hpp"

BondService::BondService(std::shared_ptr<IDatabase> primary_db,
                         std::shared_ptr<IDatabase> analytics_db)
    : primary_db_(std::move(primary_db)), analytics_db_(std::move(analytics_db)) {}

bool BondService::add_bond(const Bond& bond) {
    if (!primary_db_->save_bond(bond)) return false;
    analytics_db_->save_bond(bond);
    ensure_portfolio_exists(bond.get_rating());
    return true;
}

std::vector<Bond> BondService::get_high_yield_bonds(double threshold) {
    std::vector<Bond> result;
    return result;
}

void BondService::create_portfolio(const std::string& name) {
}

std::vector<Bond> BondService::get_portfolio_bonds(const std::string& portfolio_name) {
    std::vector<Bond> result;
    return result;
}

double BondService::calculate_portfolio_value(const std::string& portfolio_name) {
    return 0.0;
}

void BondService::ensure_portfolio_exists(const std::string& name) {
}