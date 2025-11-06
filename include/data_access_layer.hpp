// data_access_layer.hpp
#pragma once
#include "db_repository.hpp"
#include "../src/entity_config.hpp"
#include <memory>

class data_access_layer {
private:
    std::shared_ptr<db_repository> _repo;
    std::shared_ptr<entity_config> _entity_config;

public:
    data_access_layer()
        : _repo(std::make_shared<db_repository>())
        , _entity_config(std::make_shared<entity_config>()) {
    }

    // 获取数据库仓库
    std::shared_ptr<db_repository> get_repository() const {
        return _repo;
    }

    // 获取实体配置
    std::shared_ptr<entity_config> get_entity_config() const {
        return _entity_config;
    }

    // 根据实体获取数据库连接
    std::shared_ptr<db> get_db_for_entity(const std::string& entity_name) {
        auto db_name = _entity_config->get_db_for_entity(entity_name);
        return _repo->get_connection(db_name);
    }

    // 便捷方法：执行实体查询
    nlohmann::json query_entity(const std::string& entity_name, const std::string& query) {
        auto db = get_db_for_entity(entity_name);
        return db->query(query);
    }

    // 便捷方法：执行实体命令
    bool execute_for_entity(const std::string& entity_name, const std::string& command) {
        auto db = get_db_for_entity(entity_name);
        return db->execute(command);
    }
};