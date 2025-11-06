// include/db_config.hpp
#pragma once
#include <string>

struct db_config {
    std::string host = "localhost";
    int port = 0;
    std::string username;
    std::string password;
    std::string database;
    int pool_size = 5;

    // PostgreSQL 连接字符串
    std::string get_pg_connection_string() const {
        return "postgresql://" + username + ":" + password +
               "@" + host + ":" + std::to_string(port) + "/" + database;
    }

    // MongoDB 连接字符串
    std::string get_mongo_connection_string() const {
    // 例: mongodb://weiyu:secret123@localhost:27017/bond_demo?authSource=admin
    return "mongodb://" + username + ":" + password +
           "@" + host + ":" + std::to_string(port) + "/" + database +
           "?authSource=admin";
    }
};

// 默认数据库配置
namespace default_config {
    // PostgreSQL 配置
    const db_config postgresql = {
        .host = "localhost",
        .port = 5432,
        .username = "weiyu",
        .password = "secret123",
        .database = "bond_demo",
        .pool_size = 5
    };

    // MongoDB 配置
    const db_config mongodb = {
        .host = "localhost",
        .port = 27017,
        .username = "weiyu",
        .password = "secret123",
        .database = "bond_demo",
        .pool_size = 5
    };
}