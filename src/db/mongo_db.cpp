#include "../../include/db/mongo_db.hpp"

MongoDatabase::MongoDatabase() : instance_() {}

MongoDatabase::~MongoDatabase() = default;

bool MongoDatabase::connect(const DbConfig& config) {
    config_ = config;
    try {
        std::string uri = "mongodb://" + config.username + ":" + config.password +
                          "@" + config.host + ":" + std::to_string(config.port);
        client_ = mongocxx::client{mongocxx::uri{uri}};
        db_ = client_[config.database];
        collection_ = db_["bonds"];
        connected_ = true;
        return true;
    } catch (...) {
        connected_ = false;
        return false;
    }
}

bool MongoDatabase::save_bond(const Bond& bond) {
    return false;
}

bool MongoDatabase::save_bonds(const std::vector<Bond>& bonds) {
    return false;
}

std::vector<Bond> MongoDatabase::get_all_bonds() {
    return {};
}

std::vector<Bond> MongoDatabase::get_bonds_by_rating(const std::string& rating) {
    return {};
}

std::optional<Bond> MongoDatabase::get_bond_by_id(const std::string& id) {
    return std::nullopt;
}

Bond MongoDatabase::document_to_bond(const bsoncxx::document::view& doc) {
    return Bond("", "", 0.0, "", 0.0);
}