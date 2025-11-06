#include "../../include/bond/portfolio.hpp"

Portfolio::Portfolio(std::string name) : name_(std::move(name)) {}

void Portfolio::add_bond(const BondPtr& bond) {
    bonds_[bond->get_id()] = bond;
}

void Portfolio::remove_bond(const std::string& bond_id) {
    bonds_.erase(bond_id);
}

double Portfolio::calculate_total_value() const {
    double total = 0.0;
    for (const auto& [_, bond] : bonds_) {
        total += bond->get_face_value();
    }
    return total;
}

double Portfolio::calculate_average_yield() const {
    if (bonds_.empty()) return 0.0;
    double total_yield = 0.0;
    for (const auto& [_, bond] : bonds_) {
        total_yield += bond->get_yield();
    }
    return total_yield / bonds_.size();
}

std::vector<BondPtr> Portfolio::get_bonds_by_rating(const std::string& rating) const {
    std::vector<BondPtr> result;
    for (const auto& [_, bond] : bonds_) {
        if (bond->get_rating() == rating) {
            result.push_back(bond);
        }
    }
    return result;
}

std::vector<BondPtr> Portfolio::get_all_bonds() const {
    std::vector<BondPtr> result;
    for (const auto& [_, bond] : bonds_) {
        result.push_back(bond);
    }
    return result;
}

nlohmann::json Portfolio::to_json() const {
    nlohmann::json j;
    j["name"] = name_;
    j["bonds"] = nlohmann::json::array();
    for (const auto& [_, bond] : bonds_) {
        j["bonds"].push_back(bond->to_json());
    }
    return j;
}