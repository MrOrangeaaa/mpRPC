set(SRC_LIST userservice.cc ../user.pb.cc)
# set(SRC_LIST friendservice.cc ../friend.pb.cc)

add_executable(callee ${SRC_LIST})

# 注意链接顺序，mprpc依赖protobuf
target_link_libraries(callee protobuf mprpc)