set(SRC_LIST calluserservice.cc ../user.pb.cc)
# set(SRC_LIST callfriendservice.cc ../friend.pb.cc)

add_executable(caller ${SRC_LIST})

# 注意链接顺序，mprpc依赖protobuf
target_link_libraries(caller protobuf mprpc)