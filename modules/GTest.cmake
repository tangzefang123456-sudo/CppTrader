# GTest
# Download and install GTest automatically
include(ExternalProject)

set(GTEST_VERSION "1.14.0")
set(GTEST_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/GTest")
set(GTEST_INCLUDE_DIR "${GTEST_INSTALL_DIR}/include")
set(GTEST_LIBRARY_DIR "${GTEST_INSTALL_DIR}/lib")

if(MSVC)
  set(GTEST_LIBRARY "${GTEST_LIBRARY_DIR}/gtest.lib")
  set(GTEST_MAIN_LIBRARY "${GTEST_LIBRARY_DIR}/gtest_main.lib")
  set(GTEST_DEBUG_LIBRARY "${GTEST_LIBRARY_DIR}/gtestd.lib")
  set(GTEST_DEBUG_MAIN_LIBRARY "${GTEST_LIBRARY_DIR}/gtest_maind.lib")
else()
  set(GTEST_LIBRARY "${GTEST_LIBRARY_DIR}/libgtest.a")
  set(GTEST_MAIN_LIBRARY "${GTEST_LIBRARY_DIR}/libgtest_main.a")
  set(GTEST_DEBUG_LIBRARY "${GTEST_LIBRARY_DIR}/libgtestd.a")
  set(GTEST_DEBUG_MAIN_LIBRARY "${GTEST_LIBRARY_DIR}/libgtest_maind.a")
endif()

ExternalProject_Add(GTest
  URL "https://github.com/google/googletest/archive/refs/tags/v${GTEST_VERSION}.zip"
  URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${GTEST_INSTALL_DIR} -DBUILD_GMOCK=OFF -DBUILD_SHARED_LIBS=OFF
  INSTALL_DIR ${GTEST_INSTALL_DIR}
  PREFIX ${GTEST_INSTALL_DIR}
  )

# Create imported target GTest
add_library(GTest::GTest STATIC IMPORTED)
set_target_properties(GTest::GTest PROPERTIES
  IMPORTED_LOCATION ${GTEST_LIBRARY}
  IMPORTED_LOCATION_DEBUG ${GTEST_DEBUG_LIBRARY}
  INTERFACE_INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIR}
  )
add_dependencies(GTest::GTest GTest)

# Create imported target GTest::Main
add_library(GTest::Main STATIC IMPORTED)
set_target_properties(GTest::Main PROPERTIES
  IMPORTED_LOCATION ${GTEST_MAIN_LIBRARY}
  IMPORTED_LOCATION_DEBUG ${GTEST_DEBUG_MAIN_LIBRARY}
  INTERFACE_INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIR}
  )
add_dependencies(GTest::Main GTest)
