add_library(noi_est noi_est.cpp include/noi_est.hpp)
target_link_libraries(noi_est ${OpenCV_LIBS})

set_property(TARGET noi_est PROPERTY CXX_STANDARD 17)

target_include_directories(noi_est PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)