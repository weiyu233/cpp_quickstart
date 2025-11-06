// src/main.cpp
#include "data_access_layer.hpp"
#include "db_config.hpp"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>

// 加载债券数据
nlohmann::json load_bond_data() {
    std::ifstream file("data/fi_data.json");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open data/fi_data.json" << std::endl;
        return nlohmann::json::array();
    }

    nlohmann::json data;
    file >> data;
    return data;
}

int main() {
    try {
        // 创建数据访问层
        data_access_layer dal;

        // 配置数据库连接
        auto repo = dal.get_repository();
        repo->add_config("bond_master", db_type::POSTGRES, default_config::postgresql);
        repo->add_config("bond_analytics", db_type::MONGODB, default_config::mongodb);

        // 配置实体数据库映射
        auto entity_config = dal.get_entity_config();
        entity_config->register_entity("bonds", "bond_master");
        entity_config->register_entity("bond_analytics", "bond_analytics");
        entity_config->register_entity("portfolio_reports", "bond_analytics");

        // 测试连接
        if (repo->test_all_connections()) {
            std::cout << "All database connections successful!" << std::endl;
        } else {
            std::cerr << "Some database connections failed!" << std::endl;
            return 1;
        }

        // 加载债券数据
        auto bond_data = load_bond_data();
        if (bond_data.empty()) {
            std::cerr << "No bond data loaded!" << std::endl;
            return 1;
        }

        std::cout << "Loaded " << bond_data.size() << " bonds from JSON file" << std::endl;

        // 获取数据库连接
        auto bonds_db = dal.get_db_for_entity("bonds");
        auto analytics_db = dal.get_db_for_entity("bond_analytics");

        // 存储数据到两个数据库
        bonds_db->upsert_many(bond_data);
        analytics_db->upsert_many(bond_data);

        // 等待数据提交
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // PostgreSQL 查询示例
        auto pg_db = std::dynamic_pointer_cast<db_pg>(bonds_db);
        if (pg_db) {
            std::cout << "\n=== PostgreSQL Queries ===" << std::endl;

            // 查询特定投资组合
            auto eu_bonds = pg_db->get_bonds_by_portfolio("EU Core FI");
            std::cout << "EU Core FI bonds: " << eu_bonds.size() << std::endl;

            // 查询高评级债券
            auto aaa_bonds = pg_db->get_bonds_by_rating("AAA");
            std::cout << "AAA rated bonds: " << aaa_bonds.size() << std::endl;

            // 查询高收益债券
            auto high_yield = pg_db->get_high_yield_bonds(4.0);
            std::cout << "High yield bonds (>4%): " << high_yield.size() << std::endl;
        }

        // MongoDB 查询示例
        auto mongo_db = std::dynamic_pointer_cast<db_mongo>(analytics_db);
        if (mongo_db) {
            std::cout << "\n=== MongoDB Queries ===" << std::endl;

            // 查询公司债券
            auto corporate_bonds = mongo_db->get_bonds_by_portfolio("Corporate FI");
            std::cout << "Corporate bonds: " << corporate_bonds.size() << std::endl;

            // 投资组合汇总统计
            auto portfolio_summary = mongo_db->get_portfolio_summary();
            std::cout << "Portfolio summary: " << portfolio_summary.dump(2) << std::endl;
        }

        // 执行原始SQL查询 - 使用更简单的查询进行测试
        std::cout << "\n=== Testing Simple Queries ===" << std::endl;

        // 测试1: 简单计数
        auto bond_count = bonds_db->query("SELECT COUNT(*) as total_count FROM bonds");
        std::cout << "Total bonds count: " << bond_count.dump(2) << std::endl;

        // 测试2: 简单的分组查询
        auto portfolio_groups = bonds_db->query("SELECT portfolio FROM bonds GROUP BY portfolio");
        std::cout << "Portfolio groups: " << portfolio_groups.dump(2) << std::endl;

        // 测试3: 带聚合的分组查询
        auto portfolio_stats = bonds_db->query(
            "SELECT portfolio, COUNT(*) as bond_count, AVG(yield_to_maturity) as avg_yield "
            "FROM bonds GROUP BY portfolio"
        );
        std::cout << "Portfolio statistics: " << portfolio_stats.dump(2) << std::endl;

        std::cout << "\nBond data processing completed successfully!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}