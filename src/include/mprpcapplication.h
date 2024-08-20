#pragma once

#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"

// mprpc框架的基础类，负责框架的一些初始化操作
class MprpcApplication
{
public:
    // 单例模式
    static MprpcApplication& GetInstance();

    static void Init(int argc, char **argv);
    static MprpcConfig& GetConfig();

private:
    // 单例模式
    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;

private:
    static MprpcConfig m_config;
};