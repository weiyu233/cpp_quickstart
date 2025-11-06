cmake_minimum_required(VERSION 3.16)
project(cpp_quickstart VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    src/main.cpp
    src/bond/bond_service.cpp
    src/bond/portfolio.cpp
    src/db/postgres_db.cpp
    src/db/mongo_db.cpp
)

set(HEADERS
    include/bond/bond.hpp
    include/bond/bond_service.hpp
    include/bond/portfolio.hpp
    include/db/database.hpp
    include/db/postgres_db.hpp
    include/db/mongo_db.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
)

# --- nlohmann/json
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# --- libpqxx (PostgreSQL)
find_package(pqxx QUIET CONFIG)
if (pqxx_FOUND)
    message(STATUS "Using libpqxx via CMake config")
    target_link_libraries(${PROJECT_NAME} PRIVATE pqxx::pqxx)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)
    if(PQXX_FOUND)
        target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::PQXX)
    else()
        message(FATAL_ERROR "libpqxx not found - please install PostgreSQL development libraries")
    endif()
endif()

# --- mongocxx & bsoncxx
find_package(mongocxx QUIET)
find_package(bsoncxx QUIET)
if (mongocxx_FOUND AND bsoncxx_FOUND)
    message(STATUS "Using mongocxx via CMake config")
    target_link_libraries(${PROJECT_NAME} PRIVATE mongo::mongocxx_shared mongo::bsoncxx_shared)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBMONGOCXX REQUIRED libmongocxx)
    pkg_check_modules(LIBBSONCXX REQUIRED libbsoncxx)
    if(LIBMONGOCXX_FOUND AND LIBBSONCXX_FOUND)
        target_include_directories(${PROJECT_NAME} PRIVATE
            ${LIBMONGOCXX_INCLUDE_DIRS} 
            ${LIBBSONCXX_INCLUDE_DIRS}
        )
        target_link_directories(${PROJECT_NAME} PRIVATE
            ${LIBMONGOCXX_LIBRARY_DIRS} 
            ${LIBBSONCXX_LIBRARY_DIRS}
        )
        target_link_libraries(${PROJECT_NAME} PRIVATE
            ${LIBMONGOCXX_LIBRARIES} 
            ${LIBBSONCXX_LIBRARIES}
        )
        target_compile_options(${PROJECT_NAME} PRIVATE
            ${LIBMONGOCXX_CFLAGS_OTHER} 
            ${LIBBSONCXX_CFLAGS_OTHER}
        )
    else()
        message(FATAL_ERROR "mongocxx/bsoncxx not found - please install MongoDB C++ driver")
    endif()
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/data/bonds.json
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/data/bonds.json
    COMMENT "Copying bond data files to build directory"
)

enable_testing()
add_subdirectory(tests)