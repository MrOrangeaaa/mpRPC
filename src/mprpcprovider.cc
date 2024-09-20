#include "mprpcprovider.h"
#include "mprpcapplication.h"
#include "mprpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"


// 注册一个服务
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    std::string service_name = pserviceDesc->name();  // 服务名
    int method_cnt = pserviceDesc->method_count();  // 该服务提供的方法数量

    LOG_INFO("Notifying a service... [service_name: %s, method_cnt: %d]", service_name.c_str(), method_cnt);

    // 将该service的所有信息都记录到service_info中
    ServiceInfo service_info;

    // 遍历该service中的所有method
    for (int i = 0; i < method_cnt; i++)
    {
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});

        LOG_INFO("method_name: %s", method_name.c_str());
    }

    service_info.m_service = service;

    // 将该service注册到RpcProvider上
    m_serviceMap.insert({service_name, service_info});
}

// 启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run()
{
    // 读取配置文件中rpcserver相关信息
    std::string ip = MprpcApplication::GetInstance().GetConfig().Get("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Get("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");

    // 绑定连接回调和消息回调 -> 分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,
                                                this,
                                                std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage,
                                            this,
                                            std::placeholders::_1,
                                            std::placeholders::_2,
                                            std::placeholders::_3));

    // 设置muduo库的线程数量
    server.setThreadNum(4);  // 一个IO线程，三个worker线程

    // 启动网络服务
    server.start();
    std::cout << "RpcProvider start services at " << ip << ":" << port << std::endl;

    // Zookeeper -> 进行服务注册
    ZkClient zkCli;
    zkCli.Start();
    for (auto &sp : m_serviceMap) 
    {
        std::string service_node_path = "/" + sp.first;  // "/UserServiceRpc"
        zkCli.Create(service_node_path.c_str(), nullptr, 0);  // service_node为永久节点
        for (auto &mp : sp.second.m_methodMap)
        {
            std::string method_node_path = service_node_path + "/" + mp.first;  // "/UserServiceRpc/Login"
            char method_node_data[128] = {0};
            sprintf(method_node_data, "%s:%d", ip.c_str(), port);
            zkCli.Create(method_node_path.c_str(),
                        method_node_data,
                        strlen(method_node_data),
                        ZOO_EPHEMERAL);  // method_node为临时节点 -> ZOO_EPHEMERAL表示创建一个临时节点
        }
    }

    // 进入事件循环
    m_eventLoop.loop(); 
}

// Callback function -> 新的连接建立，触发回调
// 这里面其实不需要做啥...
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 客户端建立连接失败，服务端释放资源
        conn->shutdown();
    }
}

// Callback function -> 已建立的连接上发生读写事件，触发回调
// 例如：如果caller发起了一个rpc服务的调用请求，那么muduo库就会帮我们回调OnMessage()
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, 
                            muduo::net::Buffer *buffer, 
                            muduo::Timestamp)
{
    /**
    * callee和caller之间要协商好一个“通信协议” -> 这也属于RPC框架的一部分
    * mprpc中规定为: header_size(4 bytes) + header_str(service_name + method_name + args_size[用于解决tcp粘包问题]) + args_str
    */
    // 从网络上接收的远端rpc请求的字节流 -> caller肯定也是按照RPC框架约定好的通信协议组织数据发过来的
    std::string recv_str = buffer->retrieveAllAsString();

    // 读取header_size
    uint32_t header_size = 0;
    recv_str.copy((char*)&header_size, 4, 0);

    // 读取header_str，并进行反序列化
    std::string header_str = recv_str.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(header_str))
    {
        // 反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // 反序列化失败
        std::cout << "header_str parse error, content: " << header_str << std::endl;
        return;
    }

    // 读取args_str
    std::string args_str = recv_str.substr(4 + header_size, args_size);

    // // 打印调试信息
    // std::cout << "============================================" << std::endl;
    // std::cout << "header_size: " << header_size << std::endl;
    // std::cout << "header_str: " << header_str << std::endl;
    // std::cout << "service_name: " << service_name << std::endl;
    // std::cout << "method_name: " << method_name << std::endl;
    // std::cout << "args_size: " << args_size << std::endl;
    // std::cout << "args_str: " << args_str << std::endl;
    // std::cout << "============================================" << std::endl;


    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        std::cout << service_name << " does not exist!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" << method_name << " does not exist!" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service;  // 获取service对象
    const google::protobuf::MethodDescriptor *method = mit->second;  // 获取method对象


    // 创建并填充request（由RPC框架进行填充）
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        // 反序列化失败
        std::cout << "request parse error, content: " << args_str << std::endl;
        return;
    }

    // 创建response（将来callee执行完业务逻辑之后再由callee进行填充）
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();


    // 下方的CallMethod()需要我们传入一个Closure*类型的参数
    // Closure是一个抽象类，本身没什么内容，它声明了一个纯虚函数Run()
    // 我们当然要去创建一个新类(ClassXXX)，继承Closure，并重写Run() -> Run()的功能是序列化response并通过网络api发送给caller
    // 好消息是这里大部分的工作都不需要我们自己做
    // 调用模板函数NewCallback()会帮我们(在堆区上)创建一个ClassXXX实例对象，并返回该对象的指针
    // 唯独需要我们做的是：提供一个回调函数 -> 函数指针[void (Class::*method)(Arg1, Arg2)]
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                                    const muduo::net::TcpConnectionPtr&,
                                                                    google::protobuf::Message*>
                                                                    (this,
                                                                    &RpcProvider::SendRpcResponse,
                                                                    conn, response);

    // 在框架内部，根据远端rpc请求的内容，调用当前rpc节点上发布的方法
    service->CallMethod(method, nullptr, request, response, done);
}

// Callback function for Closure::Run() -> 序列化response并通过网络发送给caller
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))  // 序列化response
    {
        // 序列化成功后，通过网络将rpc方法的执行结果返回给调用方(caller)
        conn->send(response_str);
    }
    else
    {
        std::cout << "response serialize error!" << std::endl; 
    }
    conn->shutdown();  // 模拟http短连接服务，由callee(服务端)主动断开连接
}