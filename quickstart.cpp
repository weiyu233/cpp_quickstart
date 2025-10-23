#include <cstdint>
#include <iostream>
#include <vector>
#include <fstream>

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <nlohmann/json.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using nlohmann::json;

static mongocxx::instance KInstance{};

mongocxx::client& get_client() {
    const char* env_uri = std::getenv("MONGODB_URI");
    static mongocxx::client client{
        mongocxx::uri{ env_uri ? env_uri :
            "mongodb+srv://weiyu233_db_user:***REDACTED***@cluster0.jms9djc.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0"
        }
    };
    return client;
}

void read_with_pagination(const int page = 1, const int page_size = 5) {
    const auto& client = get_client();
    const auto db = client["cpp-test"];
    auto coll = db["cpp"];

    mongocxx::options::find opt;
    opt.limit(page_size);
    opt.skip((page-1) * page_size);

    auto cursor = coll.find({}, opt);
    for (auto&& doc: cursor) {
        std::cout << bsoncxx::to_json(doc) << "\n";
    }
}

void load_fi_data(mongocxx::collection& coll, const json& json_array) {
    for (const auto& item : json_array) {
        auto bond_id = item.at("bond_id").get<std::string>();

        auto doc = bsoncxx::from_json(item.dump());
        coll.update_one(
            make_document(kvp("bond_id", bond_id)),
            make_document(kvp("$set", doc.view())),
            mongocxx::options::update{}.upsert(true)
        );

        std::cout <<  "Upserted bond: " << bond_id << std::endl;
    }
}

void clear_collection(mongocxx::collection& coll) {
    if (auto result = coll.delete_many({})) {
        std::cout << "Deleted " << result->deleted_count() << " documents.\n";
    } else {
        std::cout << "Failed to delete: " << result->deleted_count() << std::endl;
    }
}

int main() {
    const auto& client = get_client();
    const auto db = client["cpp-test"];
    auto coll = db["cpp"];

    clear_collection(coll);

    std::ifstream infile("../fi_data.json");
    if (!infile.is_open()) {
        std::cerr << "Can't open file" << std::endl;
        return 1;
    }

    json json_array;
    infile >> json_array;

    load_fi_data(coll, json_array);

    std::cout << "Data load completed successfully!" << std::endl;
    return 0;
}