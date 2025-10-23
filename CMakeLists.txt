cmake_minimum_required(VERSION 3.16)
project(cpp_quickstart CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cpp_quickstart quickstart.cpp)

# --- Option A: 官方 CMake config（推荐）
find_package(mongocxx QUIET)
find_package(bsoncxx QUIET)

if (mongocxx_FOUND AND bsoncxx_FOUND)
    target_link_libraries(cpp_quickstart PRIVATE mongo::mongocxx_shared mongo::bsoncxx_shared)
else()
    message(STATUS "mongocxx/bsoncxx CMake config not found, falling back to pkg-config")

    # --- Option B: pkg-config 回退方案
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBMONGOCXX REQUIRED libmongocxx)
    pkg_check_modules(LIBBSONCXX REQUIRED libbsoncxx)

    target_include_directories(cpp_quickstart PRIVATE
            ${LIBMONGOCXX_INCLUDE_DIRS}
            ${LIBBSONCXX_INCLUDE_DIRS}
    )
    target_link_directories(cpp_quickstart PRIVATE
            ${LIBMONGOCXX_LIBRARY_DIRS}
            ${LIBBSONCXX_LIBRARY_DIRS}
    )
    target_link_libraries(cpp_quickstart PRIVATE
            ${LIBMONGOCXX_LIBRARIES}
            ${LIBBSONCXX_LIBRARIES}
    )
    target_compile_options(cpp_quickstart PRIVATE
            ${LIBMONGOCXX_CFLAGS_OTHER}
            ${LIBBSONCXX_CFLAGS_OTHER}
    )
endif()

# --- 加入 nlohmann/json
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(cpp_quickstart PRIVATE nlohmann_json::nlohmann_json)