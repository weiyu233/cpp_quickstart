#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "db_pg.hpp"
#include "db_mongo.hpp"

int main() {
    try {
        // 1) 读取数据
        std::ifstream f("data/fi_data.json");
        if (!f.is_open()) {
            std::cerr << "Can't open data/fi_data.json\n";
            return 1;
        }
        nlohmann::json arr; f >> arr;

        // 2) Postgres
        auto pg = pg_connect();
        std::cout << "Connected PG as " << pg.username() << " db=" << pg.dbname() << "\n";
        pg_ensure_schema(pg);
        pg_upsert_fi_data(pg, arr);
        pg_query_demo(pg);
        pg.close();

        // 3) Mongo
        auto coll = mongo_get_collection();
        mongo_upsert_fi_data(coll, arr);
        mongo_query_demo(coll);

        if constexpr (true) {
            mongo_query_update_many_usd_price(coll, 0.2);
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 100;
    }
}