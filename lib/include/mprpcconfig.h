#pragma once

#include <unordered_map>
#include <string>

/**
 * 无论是RPC服务的发布者(callee) or 消费者(caller)，启动程序时都需要指定RPC配置文件
 * 其中包含了以下四条信息：rpcserverip   rpcserverport   zookeeperip   zookeeperport
 */

class MprpcConfig
{
public:
    // 加载配置文件
    void LoadConfigFile(const char *config_file);
    // (在m_configMap中)查询配置项信息
    std::string Get(const std::string &key);
private:
    // 将配置文件中的信息以[key:value]的形式保存到m_configMap中
    std::unordered_map<std::string, std::string> m_configMap;
    // 去除字符串前后的空格
    void Trim(std::string &src_buf);
};