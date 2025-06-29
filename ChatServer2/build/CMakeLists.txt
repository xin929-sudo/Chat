cmake_minimum_required(VERSION 3.10...3.25)

project(YourProjectName)

#添加hiredis头文件路径
include_directories(/usr/local/include/hiredis)

# 新增：添加 hiredis 库路径
link_directories(/usr/local/lib)

# 添加 MySQL 头文件路径
include_directories(/usr/include/mysql-cppconn-8)

set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)
find_package(Threads REQUIRED)
# 设置Boost相关参数
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()


set(BOOST_ROOT "/user/include")
set(BOOST_ROOT ${Boost_ROOT})
find_package(Boost 1.73.0 REQUIRED COMPONENTS system thread filesystem)

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(_GRPC_GRPCPP gRPC::grpc++)

include_directories(../inc)
include_directories(/usr/local/lib)
aux_source_directory(../src SRC_LIST)

# 后续其他配置
add_executable(main ${SRC_LIST})
target_include_directories(main PRIVATE ${Boost_INCLUDE_DIRS} /usr/local/include/hiredis /usr/include/mysql-cppconn-8)
target_link_libraries(main PRIVATE
                    ${Boost_LIBRARIES} 
                    ${_REFLECTION}
                    ${_GRPC_GRPCPP}
                    ${_PROTOBUF_LIBPROTOBUF}
                    Threads::Threads
                    "/usr/local/lib/libjsoncpp.a"
                    hiredis
                    mysqlcppconn)
