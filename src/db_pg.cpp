#include "db_pg.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>

static std::string make_default_dsn_from_env() {
    // 从环境变量读取各字段，缺省则给默认值
    auto get = [](const char* k, const char* def) {
        const char* v = std::getenv(k);
        return std::string{ v ? v : def };
    };
    std::ostringstream dsn;
    dsn << "host="     << get("PG_HOST", "127.0.0.1")
        << " port="    << get("PG_PORT", "5432")
        << " dbname="  << get("POSTGRES_DB", "demo")
        << " user="    << get("POSTGRES_USER", "weiyu")
        << " password="<< get("POSTGRES_PASSWORD", "secret123");
    return dsn.str();
}

pqxx::connection pg_connect() {
    const char* dsn_env = std::getenv("POSTGRES_DSN");
    const std::string fallback = make_default_dsn_from_env();
    return pqxx::connection{ dsn_env ? dsn_env : fallback.c_str() };
}

void pg_ensure_schema(pqxx::connection& c) {
    pqxx::work tx{c};
    tx.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS fi_bonds (
            bond_id TEXT PRIMARY KEY,
            yield_to_maturity NUMERIC,
            duration NUMERIC,
            updated_at TIMESTAMPTZ DEFAULT now()
        )
    )SQL");
    tx.commit();
}

void pg_upsert_fi_data(pqxx::connection& c, const nlohmann::json& arr) {
    pqxx::work tx{c};
    for (const auto& item : arr) {
        const std::string id  = item.at("bond_id").get<std::string>();
        const double ytm = item.value("yield_to_maturity", 0.0);
        const double dur = item.value("duration", 0.0);

        tx.exec(
            R"SQL(
              INSERT INTO fi_bonds (bond_id, yield_to_maturity, duration)
              VALUES ($1,$2,$3)
              ON CONFLICT (bond_id) DO UPDATE
              SET yield_to_maturity=EXCLUDED.yield_to_maturity,
                  duration=EXCLUDED.duration,
                  updated_at=now()
            )SQL",
            pqxx::params{ id, ytm, dur }
        );
        std::cout << "[PG] Upserted " << id << "\n";
    }
    tx.commit();
}

void pg_query_demo(pqxx::connection& c) {
    pqxx::work tx{c};
    auto r = tx.exec(
        "SELECT bond_id, yield_to_maturity, duration "
        "FROM fi_bonds WHERE yield_to_maturity >= 3 "
        "ORDER BY duration DESC"
    );
    for (const auto& row : r) {
        std::cout << R"([PG] {"bond_id":")" << row[0].as<std::string>()
                << R"(","yield_to_maturity":)" << row[1].as<double>()
                << ",\"duration\":" << row[2].as<double>() << "}\n";
    }
}
