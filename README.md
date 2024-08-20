# mpRPC

## 简介
mpRPC框架开发基于[muduo高性能网络库] + [Protobuf]，使用[Zookeeper]完成服务发现，并提供了[异步日志系统]

## 环境要求
- 操作系统：[ubuntu 20.04]
- 编译器：[GCC 9.4.0]
- 依赖库：
  - [muduo-2.0.2]
  - [protobuf-3.19.4]
  - [zookeeper-3.4.10]
  

## 安装
### 步骤1: 安装moduo
```bash
sudo apt-get install libboost-all-dev
git clone https://github.com/chenshuo/muduo.git
cd muduo-master/
```
muduo库源码编译包含很多unit_test测试用例代码，编译耗时长，我们也用不到，于是注释掉CMakeLists.txt中这一行：
```bash
# option(MUDUO_BUILD_EXAMPLES "Build Muduo examples" ON)
```
编译并安装
```bash
./build.sh
./build.sh install
```
手动拷贝头文件和库文件到系统目录下
```bash
cd ../build/release-install-cpp11/
mv ./include/muduo/ /usr/include/
mv ./lib/* /usr/local/lib/
```

### 步骤2: 安装Protobuf
详细步骤见官方仓库
```bash
git clone https://github.com/protocolbuffers/protobuf.git
```

### 步骤3: 安装Zookeeper
```bash
cd zookeeper-3.4.10/src/c/
./configure
make
sudo make install
```
安装jdk，修改zkServer启动配置文件
```bash
sudo apt-get install default-jdk -y
cd zookeeper-3.4.10/
mkdir data
cp ./conf/zoo_sample.cfg ./conf/zoo.cfg
```
修改zoo.cfg
```bash
dataDir=/path/to/yourzkdir/zookeeper-3.4.10/data
```
启动zkServer
```bash
cd ./bin
./zkServer.sh start
```

### 步骤4: 编译mpRPC
```bash
git clone https://github.com/MrOrangeaaa/mpRPC.git
cd mpRPC-master/
./autobuild.sh
```
在lib目录下即库文件libmprpc.a，相关的头文件位于lib/include/