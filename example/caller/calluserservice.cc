#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"

int main(int argc, char **argv)
{
    // 整个程序启动以后，若想使用RPC服务，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc, argv);

    // 创建一个RPC桩(代理)对象，需要传入一个RpcChannel对象
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());

    // rpc方法的请求
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    // rpc方法的响应
    fixbug::LoginResponse response;

    // 发起rpc方法的调用（同步调用，非异步调用）
    // 底层实际上是：RpcChannel::callMethod会集中处理所有rpc方法
    MprpcController controller;
    stub.Login(&controller, &request, &response, nullptr);

    // 一次调用完成，获取调用结果
    if (controller.Failed())
    {
        std::cout << "rpc failed! error text: " << controller.ErrorText() << std::endl;
    }
    else
    {
        if (0 == response.result().errcode())
        {
            std::cout << "rpc login response success: " << response.sucess() << std::endl;
        }
        else
        {
            std::cout << "rpc login response error: " << response.result().errmsg() << std::endl;
        }
    }


    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("666666");
    fixbug::RegisterResponse rsp;

    stub.Register(nullptr, &req, &rsp, nullptr); 

    if (0 == rsp.result().errcode())
    {
        std::cout << "rpc register response success: " << rsp.sucess() << std::endl;
    }
    else
    {
        std::cout << "rpc register response error: " << rsp.result().errmsg() << std::endl;
    }
    
    return 0;
}