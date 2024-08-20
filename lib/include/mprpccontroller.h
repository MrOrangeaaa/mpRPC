#pragma once

#include <google/protobuf/service.h>
#include <string>

/**
 * 注意：RpcController仅用于caller端，包括它相关的所有错误标志、错误提示信息都仅限于caller端
 * 故RpcController只会检查caller端上发生的序列化/反序列化错误、网络IO错误等Error
 */
class MprpcController : public google::protobuf::RpcController
{
public:
    MprpcController();

    /* 重写父类中的纯虚函数 */
    // 已具体实现
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string& reason);
    // 暂时用不上
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);

private:
    bool m_failed;  // RPC错误标志
    std::string m_errText;  // RPC错误提示信息
};