#pragma once
#include <pqxx/pqxx>
#include <nlohmann/json.hpp>

pqxx::connection pg_connect();
void pg_ensure_schema(pqxx::connection& c);
void pg_upsert_fi_data(pqxx::connection& c, const nlohmann::json& arr);
void pg_query_demo(pqxx::connection& c);  // ytm>=3 ORDER BY duration DESC