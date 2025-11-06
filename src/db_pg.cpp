// db_pg.hpp
#pragma once
#include "../include/db.h"
#include <memory>
#include <nlohmann/json.hpp>

namespace pg_driver {
    class connection {
    public:
        connection(const std::string& conn_str) {}
        bool execute(const std::string& query) { return true; }
        nlohmann::json query(const std::string& query) { return nlohmann::json::array(); }
    };

    pg_driver::connection pg_connect();
    void pg_ensure_schema(pg_driver::connection& conn);
    void pg_upsert_fi_data(pg_driver::connection& conn, const nlohmann::json& arr);
}

class db_pg : public db {
public:
    db_pg() {
        _conn = std::make_unique<pg_driver::connection>(pg_driver::pg_connect());
        pg_driver::pg_ensure_schema(*_conn);
    }

    bool execute(const std::string& query) override {
        return _conn->execute(query);
    }

    nlohmann::json query(const std::string& query) override {
        return _conn->query(query);
    }

    void upsert_many(const nlohmann::json& arr) override {
        pg_driver::pg_upsert_fi_data(*_conn, arr);
    }

private:
    std::unique_ptr<pg_driver::connection> _conn;
};