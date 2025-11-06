#include "../../include/db/postgres_db.hpp"

PostgresDatabase::PostgresDatabase() = default;

bool PostgresDatabase::connect(const DbConfig& config) {
    config_ = config;
    conn_ = std::make_unique<pqxx::connection>(config.to_connection_string());
    connected_ = conn_->is_open();
    return connected_;
}

bool PostgresDatabase::save_bond(const Bond& bond) {
    try {
        pqxx::work txn(*conn_);
        txn.exec_params(
            "INSERT INTO bonds (id, name, face_value, rating, yield) "
            "VALUES ($1, $2, $3, $4, $5) "
            "ON CONFLICT (id) DO UPDATE SET "
            "name = EXCLUDED.name, "
            "face_value = EXCLUDED.face_value, "
            "rating = EXCLUDED.rating, "
            "yield = EXCLUDED.yield",
            bond.get_id(),
            bond.get_name(),
            bond.get_face_value(),
            bond.get_rating(),
            bond.get_yield()
        );
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        throw DatabaseException("Failed to save bond: " + std::string(e.what()));
    }
}

std::vector<Bond> PostgresDatabase::get_bonds_by_rating(const std::string& rating) {
    try {
        pqxx::work txn(*conn_);
        auto result = txn.exec_params(
            "SELECT * FROM bonds WHERE rating = $1",
            rating
        );

        std::vector<Bond> bonds;
        for (const auto& row : result) {
            bonds.push_back(row_to_bond(row));
        }
        return bonds;
    } catch (const std::exception& e) {
        throw DatabaseException("Failed to get bonds: " + std::string(e.what()));
    }
}

Bond PostgresDatabase::row_to_bond(const pqxx::row& row) {
    return Bond(
        row["id"].as<std::string>(),
        row["name"].as<std::string>(),
        row["face_value"].as<double>(),
        row["rating"].as<std::string>(),
        row["yield"].as<double>()
    );
}