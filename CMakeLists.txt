cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(TestProject)

# 添加可执行文件
add_executable(test test.cpp)
add_executable(connect_eRob1 connect_eRob1.cpp)
add_executable(eRobCAN_sendCOB eRobCAN_sendCOB.cpp)
# 查找并链接系统库
find_package(Threads REQUIRED)

# 链接库到可执行文件
target_link_libraries(test PRIVATE Threads::Threads ECanVci usb)
target_link_libraries(connect_eRob1 PRIVATE Threads::Threads ECanVci usb)
target_link_libraries(eRobCAN_sendCOB PRIVATE Threads::Threads ECanVci usb)