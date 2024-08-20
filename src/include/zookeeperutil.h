#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // 连接ZkServer
    void Start();
    // 在ZkServer上根据指定的path创建znode节点
    void Create(const char *path, const char *data, int datalen, int state=0);
    // 获取指定znode节点中的数据
    std::string GetData(const char *path);
private:
    // Zookeeper客户端句柄
    zhandle_t *m_zhandle;
};