#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

MprpcConfig MprpcApplication::m_config;  // 类内声明，类外初始化

// 初始化RPC框架
void MprpcApplication::Init(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout<<"format: command -i <config-file>" << std::endl;
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    while((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            std::cout<<"format: command -i <config-file>" << std::endl;
            exit(EXIT_FAILURE);
        case ':':
            std::cout<<"format: command -i <config-file>" << std::endl;
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // 加载配置文件 rpcserver_ip  rpcserver_port  zookeeper_ip  zookepper_port
    m_config.LoadConfigFile(config_file.c_str());

    // std::cout << "rpcserverip:" << m_config.Get("rpcserverip") << std::endl;
    // std::cout << "rpcserverport:" << m_config.Get("rpcserverport") << std::endl;
    // std::cout << "zookeeperip:" << m_config.Get("zookeeperip") << std::endl;
    // std::cout << "zookeeperport:" << m_config.Get("zookeeperport") << std::endl;
}

MprpcApplication& MprpcApplication::GetInstance()
{
    // 局部静态变量，仅在首次调用时初始化，后续调用时不会重新初始化
    static MprpcApplication app;
    return app;
}

MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}