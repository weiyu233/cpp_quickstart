// entity_config.hpp
#pragma once
#include <string>
#include <unordered_map>

class entity_config {
private:
    std::unordered_map<std::string, std::string> _entity_db_mapping;

public:
    // 注册实体使用的数据库
    void register_entity(const std::string& entity_name, const std::string& db_name) {
        _entity_db_mapping[entity_name] = db_name;
    }

    // 获取实体对应的数据库名称
    std::string get_db_for_entity(const std::string& entity_name) const {
        const auto it = _entity_db_mapping.find(entity_name);
        if (it == _entity_db_mapping.end()) {
            throw std::runtime_error("No database configured for entity: " + entity_name);
        }
        return it->second;
    }

    // 检查实体是否已配置
    bool is_entity_registered(const std::string& entity_name) const {
        return _entity_db_mapping.find(entity_name) != _entity_db_mapping.end();
    }
};