#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <memory>

class Bond {
public:
    Bond(std::string id, std::string name, double face_value, std::string rating, double yield)
        : id_(std::move(id)), name_(std::move(name)), face_value_(face_value),
          rating_(std::move(rating)), yield_(yield) {}

    const std::string& get_id() const { return id_; }
    const std::string& get_name() const { return name_; }
    double get_face_value() const { return face_value_; }
    const std::string& get_rating() const { return rating_; }
    double get_yield() const { return yield_; }

    nlohmann::json to_json() const {
        return {
            {"id", id_},
            {"name", name_},
            {"face_value", face_value_},
            {"rating", rating_},
            {"yield", yield_}
        };
    }

private:
    std::string id_;
    std::string name_;
    double face_value_;
    std::string rating_;
    double yield_;
};

using BondPtr = std::shared_ptr<Bond>;