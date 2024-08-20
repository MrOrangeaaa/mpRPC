#pragma once

#include "google/protobuf/service.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>

// This class is for callee -> RPC服务的提供者
class RpcProvider
{
public:
    // 注册一个服务
    void NotifyService(google::protobuf::Service *service);
    // 启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();

private:
    // Callback function -> 新的连接建立，触发回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    // Callback function -> 已建立的连接产生读写事件，触发回调
    void OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);
    // Callback function for Closure::Run() -> 序列化response并通过网络api发送给caller
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);

private:
    // 组合EventLoop
    muduo::net::EventLoop m_eventLoop;

    struct ServiceInfo
    {
        google::protobuf::Service *m_service;

        // methodMap -> 一个service中往往注册了多个method
        // 记录service中的所有method -> key:value <method_name:method_descriptor>
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };
    
    // serviceMap -> 一个RpcProvider允许提供多个service
    // 记录RpcProvider中的所有service -> key:value <service_name:service_info>
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;
};