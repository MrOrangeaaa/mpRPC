#include <iostream>
#include <string>
#include "mprpcapplication.h"
#include "mprpcprovider.h"
#include "user.pb.h"

/**
 * UserService原本是一个本地服务，提供了两个本地方法 -> Login/Register
 * 现在要将该本地服务发布成一个可供远端调用的RPC服务
 * 1. 首先要继承对应的protobuf_service
 * 2. 重写protobuf_service中的method（protobuf只帮你声明了虚函数，具体实现得你自己写）
 */
class UserService : public fixbug::UserServiceRpc  // 使用在rpc服务发布端（rpc服务提供者 -> callee）
{
public:
    /**
     * 本地方法
     */
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name:" << name << "  pwd:" << pwd << std::endl;  
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service: Register" << std::endl;
        std::cout << "id:" << id << "  name:" << name << "  pwd:" << pwd << std::endl;
        return true;
    }

    /**
     * 远端方法
     * 重写基类UserServiceRpc中的虚函数，下面这些方法都是由RPC框架自动调用的
     */
    void Login(google::protobuf::RpcController* controller,
                const fixbug::LoginRequest* request,
                fixbug::LoginResponse* response,
                google::protobuf::Closure* done)
    {
        // 1. RPC框架上报了远端调用传入的参数LoginRequest，取出参数
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 2. 调用本地方法执行业务逻辑
        bool login_result = Login(name, pwd); 

        // 3. 填充respose，通常包括错误码、错误消息、返回值等
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(login_result);

        // 4. 执行回调操作 -> 执行response的序列化和网络发送（都是由框架来完成的）
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool ret = Register(id, name, pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_sucess(ret);

        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 初始化RPC框架
    MprpcApplication::Init(argc, argv);

    RpcProvider provider;
    // 注册一个服务
    provider.NotifyService(new UserService());
    // 启动rpc服务节点，开始提供rpc远程网络调用服务
    provider.Run();

    return 0;
}