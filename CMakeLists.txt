cmake_minimum_required(VERSION 3.16)
project(cpp_dual_db CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源码
add_executable(cpp_dual_db
        src/main.cpp
        src/db_pg.cpp
        src/db_mongo.cpp
        include/db_pg.hpp
        include/db_mongo.hpp
)
target_include_directories(cpp_dual_db PRIVATE include)

# --- nlohmann/json
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(cpp_dual_db PRIVATE nlohmann_json::nlohmann_json)

# --- libpqxx（优先 CMake config，其次 pkg-config）
find_package(pqxx QUIET CONFIG)
if (pqxx_FOUND)
    message(STATUS "Using libpqxx via CMake config")
    target_link_libraries(cpp_dual_db PRIVATE pqxx::pqxx)
else()
    message(STATUS "libpqxx CMake config not found, falling back to pkg-config")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)
    target_link_libraries(cpp_dual_db PRIVATE PkgConfig::PQXX)
endif()

# --- mongocxx & bsoncxx（优先 CMake config，其次 pkg-config）
find_package(mongocxx QUIET)
find_package(bsoncxx QUIET)
if (mongocxx_FOUND AND bsoncxx_FOUND)
    message(STATUS "Using mongocxx via CMake config")
    target_link_libraries(cpp_dual_db PRIVATE mongo::mongocxx_shared mongo::bsoncxx_shared)
else()
    message(STATUS "mongocxx/bsoncxx CMake config not found, falling back to pkg-config")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBMONGOCXX REQUIRED libmongocxx)
    pkg_check_modules(LIBBSONCXX REQUIRED libbsoncxx)
    target_include_directories(cpp_dual_db PRIVATE
            ${LIBMONGOCXX_INCLUDE_DIRS} ${LIBBSONCXX_INCLUDE_DIRS})
    target_link_directories(cpp_dual_db PRIVATE
            ${LIBMONGOCXX_LIBRARY_DIRS} ${LIBBSONCXX_LIBRARY_DIRS})
    target_link_libraries(cpp_dual_db PRIVATE
            ${LIBMONGOCXX_LIBRARIES} ${LIBBSONCXX_LIBRARIES})
    target_compile_options(cpp_dual_db PRIVATE
            ${LIBMONGOCXX_CFLAGS_OTHER} ${LIBBSONCXX_CFLAGS_OTHER})
endif()

add_custom_command(TARGET cpp_dual_db POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:cpp_dual_db>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/data/fi_data.json
        $<TARGET_FILE_DIR:cpp_dual_db>/data/fi_data.json
)