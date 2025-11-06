// include/db_mongo.hpp
#pragma once

#include "db.h"
#include "db_config.hpp"

#include <memory>
#include <vector>
#include <iostream>

#include <nlohmann/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/pipeline.hpp>
#include <mongocxx/options/bulk_write.hpp>
#include <mongocxx/options/index.hpp>
#include <mongocxx/model/replace_one.hpp>

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

class db_mongo : public db {
public:
    db_mongo(const db_config& config = default_config::mongodb) {
        try {
            static mongocxx::instance instance{};
            _client = std::make_unique<mongocxx::client>(mongocxx::uri{config.get_mongo_connection_string()});
            _database = _client->database(config.database);
            ensure_bond_collections();
        } catch (const std::exception& e) {
            std::cerr << "MongoDB connection error: " << e.what() << std::endl;
            throw;
        }
    }

    bool execute(const std::string& command) override {
        try {
            auto doc = bsoncxx::from_json(command);
            auto result = _database.run_command(doc.view());
            (void)result;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "MongoDB execute error: " << e.what() << std::endl;
            return false;
        }
    }

    nlohmann::json query(const std::string& query) override {
        try {
            auto coll = _database["bonds"];
            auto query_doc = bsoncxx::from_json(query);
            auto cursor = coll.find(query_doc.view());

            nlohmann::json results = nlohmann::json::array();
            for (auto&& item : cursor) {
                results.push_back(nlohmann::json::parse(bsoncxx::to_json(item)));
            }
            return results;
        } catch (const std::exception& e) {
            std::cerr << "MongoDB query error: " << e.what() << std::endl;
            return nlohmann::json::array();
        }
    }

    void upsert_many(const nlohmann::json& data) override {
        if (!data.is_array()) return;

        try {
            auto coll = _database["bonds"];

            mongocxx::options::bulk_write bulk_opts;
            bulk_opts.ordered(false);
            auto bulk = coll.create_bulk_write(bulk_opts);

            for (const auto& bond : data) {
                // 必须有 bond_id
                auto it = bond.find("bond_id");
                if (it == bond.end() || it->is_null()) {
                    std::cerr << "Skip a bond without bond_id" << std::endl;
                    continue;
                }
                std::string bond_id = it->get<std::string>();

                // filter
                bsoncxx::builder::basic::document filter_builder;
                filter_builder.append(bsoncxx::builder::basic::kvp("bond_id", bond_id));

                // replacement (整条替换)
                auto replacement = bsoncxx::from_json(bond.dump());

                mongocxx::model::replace_one repl{filter_builder.view(), replacement.view()};
                repl.upsert(true);
                bulk.append(repl);
            }

            auto result = bulk.execute();
            if (result) {
                std::cout << "Upserted/modified "
                          << (result->upserted_count() + result->modified_count())
                          << " bonds into MongoDB" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "MongoDB upsert error: " << e.what() << std::endl;
        }
    }

    bool test_connection() override {
        try {
            return execute(R"({"ping": 1})");
        } catch (...) {
            return false;
        }
    }

    // 债券专用查询方法
    nlohmann::json get_bonds_by_portfolio(const std::string& portfolio) {
        auto query_str = R"({"portfolio": ")" + portfolio + R"("})";
        return query(query_str);
    }

    nlohmann::json get_bonds_by_rating(const std::string& rating) {
        auto query_str = R"({"credit_rating": ")" + rating + R"("})";
        return query(query_str);
    }

    nlohmann::json get_high_yield_bonds(double min_yield) {
        auto query_str = R"({"yield_to_maturity": {"$gt": )" + std::to_string(min_yield) + "}}";
        return query(query_str);
    }

    // 聚合查询示例：按投资组合统计
    nlohmann::json get_portfolio_summary() {
        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;

        try {
            auto coll = _database["bonds"];

            mongocxx::pipeline p;
            // 等价于：
            // [
            //   { $group: {
            //       _id: "$portfolio",
            //       total_positions: { $sum: "$position" },
            //       average_yield: { $avg: "$yield_to_maturity" },
            //       bond_count: { $sum: 1 }
            //   }}
            // ]
            p.group(make_document(
                kvp("_id", "$portfolio"),
                kvp("total_positions", make_document(kvp("$sum", "$position"))),
                kvp("average_yield", make_document(kvp("$avg", "$yield_to_maturity"))),
                kvp("bond_count", make_document(kvp("$sum", 1)))
            ));

            auto cursor = coll.aggregate(p);

            nlohmann::json results = nlohmann::json::array();
            for (auto&& item : cursor) {
                results.push_back(nlohmann::json::parse(bsoncxx::to_json(item)));
            }
            return results;

        } catch (const std::exception& e) {
            std::cerr << "MongoDB aggregation error: " << e.what() << std::endl;
            return nlohmann::json::array();
        }
    }

private:
    std::unique_ptr<mongocxx::client> _client;
    mongocxx::database _database;

    void ensure_bond_collections() {
        auto bonds_coll = _database["bonds"];

        // 债券ID唯一索引
        bonds_coll.create_index(
            bsoncxx::from_json(R"({"bond_id": 1})").view(),
            mongocxx::options::index{}.unique(true)
        );

        // 其他常用查询字段索引
        bonds_coll.create_index(bsoncxx::from_json(R"({"portfolio": 1})").view());
        bonds_coll.create_index(bsoncxx::from_json(R"({"credit_rating": 1})").view());
        bonds_coll.create_index(bsoncxx::from_json(R"({"yield_to_maturity": 1})").view());
        bonds_coll.create_index(bsoncxx::from_json(R"({"maturity_date": 1})").view());
    }
};
