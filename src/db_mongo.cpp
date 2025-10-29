#include "db_mongo.hpp"
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

static mongocxx::instance kInstance{};

mongocxx::client& mongo_client_singleton() {
    const char* uri = std::getenv("MONGODB_URI");
    static mongocxx::client client{
        mongocxx::uri{
            uri ? uri :
            "mongodb://appuser:appsecret@127.0.0.1:27017/cpp-test?authSource=cpp-test"
        }
    };
    return client;
}


mongocxx::collection mongo_get_collection() {
    const char* db  = std::getenv("MONGO_DB");
    const char* col = std::getenv("MONGO_COLL");
    auto& cli = mongo_client_singleton();
    return cli[ db ? db : "cpp-test" ][ col ? col : "cpp" ];
}

void mongo_upsert_fi_data(mongocxx::collection& coll, const nlohmann::json& arr) {
    for (const auto& item : arr) {
        const std::string id = item.at("bond_id").get<std::string>();
        auto doc = bsoncxx::from_json(item.dump());
        coll.update_one(
            make_document(kvp("bond_id", id)),
            make_document(kvp("$set", doc.view())),
            mongocxx::options::update{}.upsert(true)
        );
        std::cout << "[Mongo] Upserted " << id << "\n";
    }
}

void mongo_query_demo(mongocxx::collection& coll) {
    mongocxx::options::find opt;
    opt.sort(make_document(kvp("duration", -1)));
    opt.projection(make_document(kvp("_id", 0),
                                 kvp("bond_id", 1),
                                 kvp("yield_to_maturity", 1),
                                 kvp("duration", 1)));
    auto cur = coll.find(make_document(
        kvp("yield_to_maturity", make_document(kvp("$gte", 3.0)))
    ), opt);
    for (auto&& d : cur) {
        std::cout << "[Mongo] " << bsoncxx::to_json(d) << "\n";
    }
}

void mongo_query_update_many_usd_price(mongocxx::collection& coll, double delta) {
    auto result = coll.update_many(
        make_document(kvp("currency", "USD")),
        make_document(kvp("$set", make_document(kvp("price", delta))))
    );

    std::cout << "[Mongo] Matched " << result->matched_count()
        << "[Mongo] Modified " << result->modified_count() << "\n";

    auto cursor = coll.find(make_document(kvp("currency", "usd")));

    for (auto&& doc : cursor) {
        std::cout << "[Mongo] " << bsoncxx::to_json(doc) << std::endl;
    }
}