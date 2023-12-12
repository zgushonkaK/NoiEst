add_subdirectory(lib)

add_executable(test_app test.cpp)
target_link_libraries(test_app ${OpenCV_LIBS} noi_est)