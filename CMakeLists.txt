cmake_minimum_required(VERSION 3.10)
project(FattyTradersConnector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

# IB API paths - using the community-maintained twsapi
set(IB_API_ROOT "${CMAKE_SOURCE_DIR}/IBApi/twsapi")
set(IB_API_INCLUDE "${IB_API_ROOT}/cpp/client")
set(IB_API_LIB_DIR "${IB_API_ROOT}/cpp/.libs")

# Include directories
include_directories(${IB_API_INCLUDE})
include_directories("${CMAKE_SOURCE_DIR}/src")

# Source files for console app
set(CONSOLE_SOURCES
    src/main.cpp
    src/IBConnector.cpp
    src/TradingApp.cpp
)

# Enable Qt MOC (Meta-Object Compiler) for GUI
set(CMAKE_AUTOMOC ON)

# Source files for GUI app (now with IB API integration)
set(GUI_SOURCES
    src/main_gui.cpp
    src/IBConnector.cpp
    src/ConnectionStatusGUI.cpp
    src/ConnectionStatusGUI.h
)

# Add executables
add_executable(fatty_traders ${CONSOLE_SOURCES})
add_executable(fatty_traders_gui ${GUI_SOURCES})

# Link libraries for console app
target_link_libraries(fatty_traders 
    Threads::Threads
    ${IB_API_LIB_DIR}/libtwsapi.a
)

# Link libraries for GUI app (now with IB API integration)
target_link_libraries(fatty_traders_gui 
    Threads::Threads
    Qt5::Core
    Qt5::Widgets
    ${IB_API_LIB_DIR}/libtwsapi-iborig.a
)

# Compiler flags for macOS
if(APPLE)
    target_compile_definitions(fatty_traders PRIVATE IB_USE_STD_STRING)
    target_compile_definitions(fatty_traders_gui PRIVATE IB_USE_STD_STRING)
    
    set_target_properties(fatty_traders PROPERTIES
        MACOSX_RPATH TRUE
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    
    set_target_properties(fatty_traders_gui PROPERTIES
        MACOSX_RPATH TRUE
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

# Copy config files
configure_file(${CMAKE_SOURCE_DIR}/config/settings.json ${CMAKE_BINARY_DIR}/settings.json COPYONLY)