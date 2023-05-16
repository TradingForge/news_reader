cmake_minimum_required(VERSION 3.1)
project(cpp_infra)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/bin")
set(PACKAGE_DIR "$ENV{HOME}/.news_watcher")

file(MAKE_DIRECTORY "${PACKAGE_DIR}")

file(MAKE_DIRECTORY "${PACKAGE_DIR}")
message("CONFIG DIRECTORY PATH: ${PACKAGE_DIR}/")

if (NOT DEFINED TESTWATCH_ACTIVATOR_FILE)
    set(TESTWATCH_ACTIVATOR_FILE "${PACKAGE_DIR}/testwatch_activator.py")
endif ()

if (NOT DEFINED CONFIG_FILE)
    set(CONFIG_FILE "${PACKAGE_DIR}/config.json")
endif ()

if (NOT DEFINED CPI_TEST_DATA_DIR)
    set(CPI_TEST_DATA_DIR "${PACKAGE_DIR}/cpi_test_data")
endif ()

file(MAKE_DIRECTORY "${CPI_TEST_DATA_DIR}")
message("CPI TEST DATA DIRECTORY PATH: ${CPI_TEST_DATA_DIR}/")

if (NOT DEFINED LOGS_DIR)
    set(LOGS_DIR "${PACKAGE_DIR}/logs")
endif ()

file(MAKE_DIRECTORY "${LOGS_DIR}")
message("LOGS DIRECTORY PATH: ${LOGS_DIR}")

if (NOT DEFINED TEST_LOGS_DIR)
    set(TEST_LOGS_DIR "${PACKAGE_DIR}/test_logs")
endif ()

file(MAKE_DIRECTORY "${TEST_LOGS_DIR}")
message("TEST LOGS DIRECTORY PATH: ${TEST_LOGS_DIR}")

find_package(CURL REQUIRED)
find_package(cJSON REQUIRED)    
find_package(fmt REQUIRED)

add_executable(news_watcher main.cpp
        Abstract/EndpointListener.hpp 
        Utility/Stopwatch.hpp Utility/HttpSession.hpp Utility/Logging.hpp
        Listeners/Cpi/CpiHtmEndpointListener.hpp Listeners/Cpi/CpiApiEndpointListener.hpp Listeners/Cpi/CpiRssEndpointListener.hpp Listeners/Cpi/CpiPdfEndpointListener.hpp
        Listeners/PerformanceEndpointListener.hpp Utility/Configuration.hpp Utility/TcpServer.hpp Utility/SignalHandler.hpp)

target_link_libraries(news_watcher PRIVATE cjson)
target_link_libraries(news_watcher PRIVATE CURL::libcurl)
target_link_libraries(news_watcher PRIVATE fmt::fmt-header-only)

find_library(P7 P7x64d P7x32d libP7d.a libP7d P7x64 P7x32 libP7.a libP7 REQUIRED NO_CACHE)
message("P7 LIBRARY FILE: ${P7}")
target_link_libraries(news_watcher PUBLIC ${P7})

target_compile_definitions(news_watcher PRIVATE 
        __LOGS_DIRECTORY_PATH="${LOGS_DIR}"
        __TEST_LOGS_DIRECTORY_PATH="${TEST_LOGS_DIR}"
        __CONFIGURATION_FILE_PATH="${CONFIG_FILE}"
        __TESTWATCH_ACTIVATOR_FILE_PATH="${TESTWATCH_ACTIVATOR_FILE}"
        __CPI_TEST_DATA_DIRECTORY_PATH="${CPI_TEST_DATA_DIR}")

file(COPY ${CMAKE_SOURCE_DIR}/CpiTestData/cpi_htm_test.dat DESTINATION "${CPI_TEST_DATA_DIR}")
file(COPY ${CMAKE_SOURCE_DIR}/CpiTestData/cpi_rss_test.dat DESTINATION "${CPI_TEST_DATA_DIR}")
file(COPY ${CMAKE_SOURCE_DIR}/CpiTestData/cpi_api_test.dat DESTINATION "${CPI_TEST_DATA_DIR}")
file(COPY ${CMAKE_SOURCE_DIR}/config.json DESTINATION "${PACKAGE_DIR}")
file(COPY ${CMAKE_SOURCE_DIR}/testwatch_activator.py DESTINATION "${PACKAGE_DIR}")
 
install(TARGETS news_watcher DESTINATION /bin)
