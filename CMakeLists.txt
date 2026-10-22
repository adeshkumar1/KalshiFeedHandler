cmake_minimum_required(VERSION 3.20)

project(kalshi-feed-handler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL 3.0 REQUIRED COMPONENTS Crypto SSL)
find_package(Boost 1.70 REQUIRED CONFIG)
find_package(Threads REQUIRED)

include(FetchContent)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(kalshi-feed-handler
    src/main.cpp
    src/KalshiClient.cpp
    src/OrderBook.cpp
)

target_include_directories(kalshi-feed-handler PRIVATE include)
target_link_libraries(kalshi-feed-handler PRIVATE
    OpenSSL::Crypto
    OpenSSL::SSL
    Boost::headers
    Threads::Threads
    nlohmann_json::nlohmann_json
)
