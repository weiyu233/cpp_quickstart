#pragma once
#include "database.hpp"
#include <pqxx/pqxx>
#include <memory>

class PostgresDatabase : public IDatabase {
public:
    PostgresDatabase();
    ~PostgresDatabase() override;

    bool connect(const DbConfig& config) override;
    void disconnect() override;
    bool is_connected() const override;
    bool test_connection() override;

    bool save_bond(const Bond& bond) override;
    bool save_bonds(const std::vector<Bond>& bonds) override;
    std::vector<Bond> get_all_bonds() override;
    std::vector<Bond> get_bonds_by_rating(const std::string& rating) override;
    std::optional<Bond> get_bond_by_id(const std::string& id) override;

private:
    DbConfig config_;
    bool connected_ = false;
    std::unique_ptr<pqxx::connection> conn_;

    Bond row_to_bond(const pqxx::row& row);
};