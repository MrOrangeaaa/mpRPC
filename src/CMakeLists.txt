#aux_source_directory(. SRC_LIST)
set(SRC_LIST
    mprpcapplication.cc 
    mprpcconfig.cc 
    mprpcheader.pb.cc 
    mprpcprovider.cc 
    mprpcchannel.cc
    mprpccontroller.cc
    logger.cc
    zookeeperutil.cc)

# 这里选择生成静态库，而非动态库，原因在于编译安装muduo库的时候默认是编译成静态库
# 倘若想把mprpc框架弄成一个动态库libmprpc.so，但是链接阶段却要链接静态的muduo库是行不通的...
# 除非你把muduo库也编译成动态库
#add_library(mprpc SHARED ${SRC_LIST})
add_library(mprpc ${SRC_LIST})

# 注意链接的先后顺序，必须先muduo_net，后muduo_base -> 库之间是有依赖关系的
# 我们使用的是多线程版本的Zookeeper库 -> zookeeper_mt
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)