#include "../include/bond/bond_service.hpp"
#include "../include/db/postgres_db.hpp"
#include "../include/db/mongo_db.hpp"
#include <iostream>

int main() {
    try {
        DbConfig pg_config{"localhost", 5432, "bonds_db", "postgres", "password"};
        DbConfig mongo_config{"localhost", 27017, "bonds_analytics", "admin", "password"};

        auto primary_db = std::make_shared<PostgresDatabase>();
        auto analytics_db = std::make_shared<MongoDatabase>();

        primary_db->connect(pg_config);
        analytics_db->connect(mongo_config);

        BondService bond_service(primary_db, analytics_db);

        std::vector<Bond> sample_bonds = {
            Bond("UST001", "US Treasury 10Y", 1000.0, "AAA", 3.5),
            Bond("CORP001", "Apple Inc 5Y", 1000.0, "AA+", 4.2),
            Bond("GOV001", "German Bund 5Y", 1000.0, "AAA", 2.8)
        };

        for (const auto& bond : sample_bonds) {
            if (!bond_service.add_bond(bond)) {
                std::cerr << "Failed to add bond: " << bond.get_id() << "\n";
            }
        }

        auto high_yield_bonds = bond_service.get_high_yield_bonds(3.0);
        std::cout << "Found " << high_yield_bonds.size() << " high yield bonds\n";

        auto portfolio_value = bond_service.calculate_portfolio_value("AAA");
        std::cout << "AAA portfolio value: " << portfolio_value << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}