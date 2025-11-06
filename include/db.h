// db.h
#pragma once
#include <string>
#include <nlohmann/json.hpp>

class db {
public:
    virtual ~db() = default;

    virtual bool execute(const std::string& query) = 0;

    virtual nlohmann::json query(const std::string& query) = 0;

    virtual void upsert_many(const nlohmann::json& data) = 0;

    virtual bool test_connection() = 0;
};