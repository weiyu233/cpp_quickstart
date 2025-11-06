#pragma once
#include "../bond/bond.hpp"
#include <vector>
#include <optional>
#include <string>

struct DbConfig {
    std::string host;
    int port;
    std::string database;
    std::string username;
    std::string password;

    std::string to_connection_string() const {
        return "host=" + host +
               " port=" + std::to_string(port) +
               " dbname=" + database +
               " user=" + username +
               " password=" + password;
    }
};

class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const std::string& msg) : std::runtime_error(msg) {}
};

class IDatabase {
public:
    virtual ~IDatabase() = default;

    virtual bool connect(const DbConfig& config) = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
    virtual bool test_connection() = 0;

    virtual bool save_bond(const Bond& bond) = 0;
    virtual bool save_bonds(const std::vector<Bond>& bonds) = 0;
    virtual std::vector<Bond> get_all_bonds() = 0;
    virtual std::vector<Bond> get_bonds_by_rating(const std::string& rating) = 0;
    virtual std::optional<Bond> get_bond_by_id(const std::string& id) = 0;
};