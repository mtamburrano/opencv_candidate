find_package(libg2o QUIET)

if (NOT ${libg2o_FOUND})
return()
endif()

find_package(OpenCV REQUIRED)

# Model capture stuff
find_package(PCL REQUIRED COMPONENTS io filters visualization features segmentation surface)
include_directories(SYSTEM ${PCL_INCLUDE_DIRS}
                           /usr/include/suitesparse
)

# Find Eigen3
FIND_PACKAGE(Eigen)
find_package(libg2o)

include_directories(SYSTEM ${EIGEN_INCLUDE_DIRS})

# add reconst3d library
file(GLOB reconst3d_sources "*.cpp")

add_library(opencv_candidate_reconst3d ${reconst3d_sources})
target_link_libraries(opencv_candidate_reconst3d ${catkin_LIBRARIES}
                                                 ${OpenCV_LIBRARIES}
                                                 opencv_rgbd
                                                 ${libg2o_LIBRARIES}
                                                 ${PCL_LIBRARIES}
                                                 cholmod
                                                 cxsparse
)
set_target_properties(opencv_candidate_reconst3d PROPERTIES COMPILE_FLAGS "-fopenmp" LINK_FLAGS "-fopenmp")

# add model capture sample
add_executable(model_capture samples/model_capture.cpp)
target_link_libraries(model_capture ${OpenCV_LIBRARIES}
                                    opencv_rgbd
                                    opencv_candidate_reconst3d
)
set_target_properties(model_capture PROPERTIES COMPILE_FLAGS "-fopenmp" LINK_FLAGS "-fopenmp")
