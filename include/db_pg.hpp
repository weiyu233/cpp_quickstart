// include/db_pg.hpp
#pragma once

#include "db.h"
#include "db_config.hpp"

#include <memory>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <iostream>
#include <sstream>
#include <optional>

class db_pg : public db {
public:
    db_pg(const db_config& config = default_config::postgresql) {
        try {
            _conn = std::make_unique<pqxx::connection>(config.get_pg_connection_string());
            ensure_bond_schema();
        } catch (const std::exception& e) {
            std::cerr << "PostgreSQL connection error: " << e.what() << std::endl;
            throw;
        }
    }

    bool execute(const std::string& sql) override {
        try {
            pqxx::work txn{*_conn};
            txn.exec(sql);
            txn.commit();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "PostgreSQL execute error: " << e.what() << std::endl;
            return false;
        }
    }

    nlohmann::json query(const std::string& sql) override {
        try {
            pqxx::work txn{*_conn};
            auto result = txn.exec(sql);
            return bond_result_to_json(result);
        } catch (const std::exception& e) {
            std::cerr << "PostgreSQL query error: " << e.what() << std::endl;
            return nlohmann::json::array();
        }
    }

    void upsert_many(const nlohmann::json& data) override {
        if (!data.is_array()) return;

        try {
            pqxx::work txn{*_conn};

            for (const auto& bond : data) {
                std::string sql = build_bond_upsert_query(txn, bond);
                txn.exec(sql);
            }

            txn.commit();
            std::cout << "Upserted " << data.size() << " bonds into PostgreSQL" << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "PostgreSQL upsert error: " << e.what() << std::endl;
        }
    }

    bool test_connection() override {
        try {
            return execute("SELECT 1");
        } catch (...) {
            return false;
        }
    }

    // 债券专用查询方法
    nlohmann::json get_bonds_by_portfolio(const std::string& portfolio) {
        // 用 this->query，避免名字遮蔽
        std::stringstream ss;
        ss << "SELECT * FROM bonds WHERE portfolio = " << pqxx::work{*_conn}.quote(portfolio);
        return query(ss.str());
    }

    nlohmann::json get_bonds_by_rating(const std::string& rating) {
        std::stringstream ss;
        ss << "SELECT * FROM bonds WHERE credit_rating = " << pqxx::work{*_conn}.quote(rating);
        return query(ss.str());
    }

    nlohmann::json get_high_yield_bonds(double min_yield) {
        std::stringstream ss;
        ss << "SELECT * FROM bonds WHERE yield_to_maturity > " << min_yield;
        return query(ss.str());
    }

private:
    std::unique_ptr<pqxx::connection> _conn;

    void ensure_bond_schema() {
        try {
            // 创建债券专用表
            execute(R"(
                CREATE TABLE IF NOT EXISTS bonds (
                    bond_id VARCHAR(50) PRIMARY KEY,
                    issuer TEXT NOT NULL,
                    coupon DECIMAL(5,2),
                    maturity_date DATE,
                    issue_date DATE,
                    price DECIMAL(8,2),
                    yield_to_maturity DECIMAL(5,2),
                    duration DECIMAL(4,1),
                    credit_rating VARCHAR(10),
                    currency VARCHAR(10),
                    position BIGINT,
                    portfolio TEXT,
                    last_update TIMESTAMP,
                    created_at TIMESTAMP DEFAULT NOW(),
                    updated_at TIMESTAMP DEFAULT NOW()
                )
            )");

            // 创建索引以提高查询性能
            execute("CREATE INDEX IF NOT EXISTS idx_bonds_portfolio ON bonds(portfolio)");
            execute("CREATE INDEX IF NOT EXISTS idx_bonds_rating ON bonds(credit_rating)");
            execute("CREATE INDEX IF NOT EXISTS idx_bonds_maturity ON bonds(maturity_date)");
            execute("CREATE INDEX IF NOT EXISTS idx_bonds_yield ON bonds(yield_to_maturity)");

            std::cout << "Bond schema ensured successfully." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to ensure bond schema: " << e.what() << std::endl;
        }
    }

    // 安全读取一列为 string
    static std::optional<std::string> get_opt_string(const pqxx::row& row, const char* col) {
        if (row[col].is_null()) return std::nullopt;
        return std::string(row[col].c_str());
    }

    nlohmann::json bond_result_to_json(const pqxx::result& result) {
        nlohmann::json json_array = nlohmann::json::array();
        for (const auto& row : result) {
            nlohmann::json bond;

            // 必有的
            bond["bond_id"] = row["bond_id"].c_str();
            bond["issuer"] = row["issuer"].c_str();

            // 可为 NULL 的都先判断
            if (!row["coupon"].is_null())             bond["coupon"] = row["coupon"].as<double>();
            if (!row["maturity_date"].is_null())      bond["maturity_date"] = row["maturity_date"].c_str();
            if (!row["issue_date"].is_null())         bond["issue_date"] = row["issue_date"].c_str();
            if (!row["price"].is_null())              bond["price"] = row["price"].as<double>();
            if (!row["yield_to_maturity"].is_null())  bond["yield_to_maturity"] = row["yield_to_maturity"].as<double>();
            if (!row["duration"].is_null())           bond["duration"] = row["duration"].as<double>();
            if (!row["credit_rating"].is_null())      bond["credit_rating"] = row["credit_rating"].c_str();
            if (!row["currency"].is_null())           bond["currency"] = row["currency"].c_str();
            if (!row["position"].is_null())           bond["position"] = row["position"].as<long long>();
            if (!row["portfolio"].is_null())          bond["portfolio"] = row["portfolio"].c_str();
            if (!row["last_update"].is_null())        bond["last_update"] = row["last_update"].c_str();
            if (!row["created_at"].is_null())         bond["created_at"] = row["created_at"].c_str();
            if (!row["updated_at"].is_null())         bond["updated_at"] = row["updated_at"].c_str();

            json_array.push_back(bond);
        }
        return json_array;
    }

    // 根据 json 构造 INSERT ... ON CONFLICT ... DO UPDATE
    std::string build_bond_upsert_query(pqxx::work& txn, const nlohmann::json& bond) {
        // 这些字段我假设必有：bond_id, issuer
        const auto& bond_id = bond.at("bond_id").get_ref<const std::string&>();
        const auto& issuer  = bond.at("issuer").get_ref<const std::string&>();

        // 可选字段安全取一下
        auto get_str = [&](const char* key) -> std::string {
            if (bond.contains(key) && !bond[key].is_null())
                return txn.quote(bond[key].get<std::string>());
            return "NULL";
        };
        auto get_num = [&](const char* key) -> std::string {
            if (bond.contains(key) && !bond[key].is_null())
                return bond[key].dump(); // 数字直接dump
            return "NULL";
        };

        std::stringstream ss;
        ss << "INSERT INTO bonds ("
           << "bond_id, issuer, coupon, maturity_date, issue_date, price, "
           << "yield_to_maturity, duration, credit_rating, currency, position, portfolio, last_update"
           << ") VALUES ("
           << txn.quote(bond_id) << ", "
           << txn.quote(issuer) << ", "
           << get_num("coupon") << ", "
           << get_str("maturity_date") << ", "
           << get_str("issue_date") << ", "
           << get_num("price") << ", "
           << get_num("yield_to_maturity") << ", "
           << get_num("duration") << ", "
           << get_str("credit_rating") << ", "
           << get_str("currency") << ", "
           << get_num("position") << ", "
           << get_str("portfolio") << ", "
           << get_str("last_update")
           << ") ON CONFLICT (bond_id) DO UPDATE SET "
           << "issuer = EXCLUDED.issuer, "
           << "coupon = EXCLUDED.coupon, "
           << "maturity_date = EXCLUDED.maturity_date, "
           << "issue_date = EXCLUDED.issue_date, "
           << "price = EXCLUDED.price, "
           << "yield_to_maturity = EXCLUDED.yield_to_maturity, "
           << "duration = EXCLUDED.duration, "
           << "credit_rating = EXCLUDED.credit_rating, "
           << "currency = EXCLUDED.currency, "
           << "position = EXCLUDED.position, "
           << "portfolio = EXCLUDED.portfolio, "
           << "last_update = EXCLUDED.last_update, "
           << "updated_at = NOW()";

        return ss.str();
    }
};
