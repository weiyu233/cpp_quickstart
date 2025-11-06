#pragma once
#include "db.h"
#include "db_pg.hpp"
#include "db_mongo.hpp"
#include "db_config.hpp"
#include <memory>
#include <unordered_map>
#include <string>

enum class db_type {
    POSTGRES,
    MONGODB
};

class db_repository {
private:
    std::unordered_map<std::string, std::shared_ptr<db>> _connections;
    std::unordered_map<std::string, std::pair<db_type, db_config>> _configs;

public:
    // 添加数据库配置
    void add_config(const std::string& name, db_type type, const db_config& config = {}) {
        _configs[name] = {type, config};
    }

    // 获取数据库连接
    std::shared_ptr<db> get_connection(const std::string& name) {
        auto it = _connections.find(name);
        if (it != _connections.end()) {
            return it->second;
        }

        // 创建新连接
        auto config_it = _configs.find(name);
        if (config_it == _configs.end()) {
            throw std::runtime_error("Database configuration not found: " + name);
        }

        std::shared_ptr<db> connection;
        const auto& [type, config] = config_it->second;

        switch (type) {
            case db_type::POSTGRES: {
                auto pg_config = config;
                if (pg_config.port == 0) pg_config.port = 5432;
                connection = std::make_shared<db_pg>(pg_config);
                break;
            }
            case db_type::MONGODB: {
                auto mongo_config = config;
                if (mongo_config.port == 0) mongo_config.port = 27017;
                connection = std::make_shared<db_mongo>(mongo_config);
                break;
            }
            default:
                throw std::runtime_error("Unsupported database type");
        }

        _connections[name] = connection;
        return connection;
    }

    // 测试所有连接
    bool test_all_connections() {
        for (const auto& [name, conn] : _connections) {
            if (!conn->test_connection()) {
                return false;
            }
        }
        return true;
    }
};