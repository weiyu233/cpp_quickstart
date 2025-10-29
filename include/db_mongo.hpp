#pragma once
#include <mongocxx/client.hpp>
#include <nlohmann/json.hpp>

mongocxx::client& mongo_client_singleton();
mongocxx::collection mongo_get_collection();
void mongo_upsert_fi_data(mongocxx::collection& coll, const nlohmann::json& arr);
void mongo_query_demo(mongocxx::collection& coll);  // ytm>=3, sort duration desc, project
void mongo_query_update_many_usd_price(mongocxx::collection& coll, double delta);