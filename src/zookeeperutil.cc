#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>

// 回调函数
void zk_callback(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)  // 会话事件
	{
		if (state == ZOO_CONNECTED_STATE)  // ZkClient与ZkServer已成功建立连接
		{
			sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
		}
	}
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);  // 关闭句柄，释放资源
    }
}

// 连接ZkServer
void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().GetConfig().Get("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Get("zookeeperport");
    std::string connstr = host + ":" + port;
    
	/**
	 * zookeeper_mt: 多线程版本，三个线程并行工作
	 * 1. API调用线程
	 * 2. 网络IO线程 -> pthread_create  poll
	 * 3. zk_callback回调线程 -> pthread_create
	 */
    m_zhandle = zookeeper_init(connstr.c_str(), zk_callback, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle) 
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);

    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem);  // zookeeper_init()与ZkServer建立连接是异步进行的，这里阻塞等待连接成功
    std::cout << "connect to ZkServer success!" << std::endl;
}

// 根据指定的path创建znode节点
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int ret;

	// 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
	ret = zoo_exists(m_zhandle, path, 0, nullptr);
	if (ret == ZNONODE)  // 表示path的znode节点不存在
	{
		// 创建指定path的znode节点了
		ret = zoo_create(m_zhandle, path, data, datalen,
						&ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
		if (ret == ZOK)
		{
			std::cout << "znode create success... path:" << path << std::endl;
		}
		else
		{
			std::cout << "zk error:" << ret << std::endl;
			std::cout << "znode create error... path:" << path << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

// 获取指定znode节点中的数据
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
	int bufferlen = sizeof(buffer);
	int ret = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
	if (ret != ZOK)
	{
		std::cout << "zk error:" << ret << std::endl;
		std::cout << "znode get error... path:" << path << std::endl;
		return "";
	}
	else
	{
		return buffer;
	}
}