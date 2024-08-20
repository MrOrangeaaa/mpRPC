#include "mprpcchannel.h"
#include "mprpcheader.pb.h"
#include "zookeeperutil.h"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

// 所有通过<stub代理对象>调用的rpc方法，都走到这里了，统一做数据序列化和网络发送 
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                                google::protobuf::RpcController* controller,
                                const google::protobuf::Message* request,
                                google::protobuf::Message* response,
                                google::protobuf:: Closure* done)
{
    /**
     * RpcProvider和RpcConsumer之间约定好的应用层协议：
     * header_size(4 bytes) + header_str(service_name + method_name + args_size) + args_str
     */
    
    const google::protobuf::ServiceDescriptor* sdp = method->service();
    std::string service_name = sdp->name();  // service_name
    std::string method_name = method->name();  // method_name

    // args_str args_size
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("serialize to args_str error!");
        return;
    }
    
    // 定义rpc的请求header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    // header_str header_size
    uint32_t header_size = 0;
    std::string header_str;
    if (rpcHeader.SerializeToString(&header_str))
    {
        header_size = header_str.size();
    }
    else
    {
        controller->SetFailed("serialize to header_str error!");
        return;
    }

    // 组织待发送的rpc请求字符串 -> xxx_str就是序列化之后的数据，所谓“序列化”就是将一个结构体组织成一个字符串
    std::string send_str;
    send_str.insert(0, std::string((char*)&header_size, 4));
    send_str += header_str;
    send_str += args_str;

    // // 打印调试信息
    // std::cout << "============================================" << std::endl;
    // std::cout << "header_size: " << header_size << std::endl;
    // std::cout << "header_str: " << header_str << std::endl;
    // std::cout << "service_name: " << service_name << std::endl;
    // std::cout << "method_name: " << method_name << std::endl;
    // std::cout << "args_size: " << args_size << std::endl;
    // std::cout << "args_str: " << args_str << std::endl;
    // std::cout << "============================================" << std::endl;


    /**
     * 以下是网络编程的部分 -> 直接使用POSIX API即可，无需使用网络库
     */
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "socket() error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 服务发现
    /*------ Version 1.0 - 从配置文件中获取rpc服务节点的ip:port ------*/
    // std::string ip = MprpcApplication::GetInstance().GetConfig().Get("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Get("rpcserverport").c_str());
    /*------ Version 2.0 - 在Zookeeper(服务注册中心)上查询目标method所在的rpc服务节点的ip:port ------*/
    ZkClient zkCli;
    zkCli.Start();
    std::string method_node_path = "/" + service_name + "/" + method_name;
    std::string data = zkCli.GetData(method_node_path.c_str());
    if (data == "")
    {
        controller->SetFailed(method_node_path + " does not exist!");
        return;
    }
    int idx = data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_node_path + " data is invalid!");
        return;
    }
    std::string ip = data.substr(0, idx);
    uint16_t port = atoi(data.substr(idx + 1, data.size() - idx).c_str()); 

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接rpc服务节点
    if (-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "connect() error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 发送rpc请求
    if (-1 == send(clientfd, send_str.c_str(), send_str.size(), 0))
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "send() error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 接收rpc响应
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))  // 阻塞IO
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "recv() error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 反序列化响应数据
    /*------ Version 1.0 ------*/
    // std::string response_str(recv_buf, 0, recv_size);  // Bug: 构造response_str时，若在recv_buf中出现"\0"，后面的数据就会被丢弃
    // if (!response->ParseFromString(response_str))
    /*------ Version 2.0 ------*/
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "deserialize to response error!");
        controller->SetFailed(errtxt);
        return;
    }

    close(clientfd);
}