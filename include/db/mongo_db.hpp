#pragma once
#include "database.hpp"
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/document/view.hpp>

class MongoDatabase : public IDatabase {
public:
    MongoDatabase();
    ~MongoDatabase() override;

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
    mongocxx::instance instance_;
    mongocxx::client client_;
    mongocxx::database db_;
    mongocxx::collection collection_;

    Bond document_to_bond(const bsoncxx::document::view& doc); 
};