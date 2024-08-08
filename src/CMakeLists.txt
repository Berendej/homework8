cmake_minimum_required( VERSION 3.10 )
project( bayan VERSION 0.1.1 )

find_package(Boost COMPONENTS program_options filesystem unit_test_framework regex REQUIRED)

add_executable( bayan main.cpp comparer.cpp phile.cpp )
add_executable( bayan_test test_bayan.cpp comparer.cpp phile.cpp )

target_link_libraries( bayan ${Boost_LIBRARIES})
target_link_libraries( bayan_test ${Boost_LIBRARIES})

# package part which enales
# run: cmake --build . --target package
install(TARGETS bayan RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
set(CPACK_PACKAGE_CONTACT berrendej@gmail.com)
include(CPack)

#testing part which enables
# cmake --build . --target test
enable_testing()
add_test(bayan_test bayan_test) 
