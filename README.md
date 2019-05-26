libevwork 轻量级C++网络引擎库
==========================================================

### 项目说明：
之所以想写这个库，源于2015年，当时我在长沙一家手游戏公司做后端服务。公司原有的服务器代码，虽然基于知名的开源事件库libev，但是所有的底层函数和业务逻辑全部耦合在一个大文件里，动辄几千上万行的代码，基本上没有明显的层次关系，开发质量经常得不到保障。签于此，我仍然基于libev，并结合以前我对网络引擎的理解，编写了这个库，并应用于团队后续的项目开发中，目前已有3年多时间。

libevwork的名称也有此而来，基于libev底层事件库，实现更高一层的连接管理和收发缓冲管理，并且提供多种应用层协议支持。它整体的工作流程是这样的：<br>
libev事件 -> 连接管理 -> 收发包缓冲 -> 应用层协议解包 -> 消息队列 -> 消息分派 -> 消息处理。<br>
用好libevwork，使用者不需要过多的关注底层（libev及至以上各流程）的实现细节，只需要简单定义好消息结构，并注册好消息回调函数既可，并且提供友好的MFC编程风格。

----------------------------------------------------------
### 编译说明：

编译libevwork前，需要安装以下依赖库：
1. libev
2. libboost-dev
3. protobuf
4. jsoncpp

##### libev的安装：

源码安装：<br>
wget http://dist.schmorp.de/libev/libev-4.15.tar.gz <br>
tar -zxvf libev-4.15.tar.gz <br>
cd libev-4.15 <br>
./configure <br>
make <br>
sudo make install<br>

包安装：<br>
centos下也可以直接yum安装：<br>
sudo yum install libev-devel <br>

##### libboost-dev的安装：

包安装：<br>
ubuntu -> sudo apt-get install libboost-dev <br>
centos -> sudo yum install boost-devel <br>

有可能默认源的版本与需要的不符，这时请登录官网下载需要的版本，网址：<br>
http://sourceforge.net/projects/boost/files/ <br>
 
编译安装：<br>
首先下载源码，解压，进行源码根目录，然后执行：<br>
 
1. 创建boost自己的编译工具bjam，执行：<br>
./bootstrap.sh
注：也可以使用前缀选项指定安装目录，如
./bootstrap.sh --prefix=/home/kdjie/boost_1_43_0/boost_install，不指定prefix默认安装到/usr/local/include和/usr/local/lib。
 
2. 编译boost，执行：<br>
./bjam <br>
或 <br>
./bjam -a 重新编译 <br>
默认生成的中间文件在./bin.v2目录，所有的库会被收集在stage/lib目录下。
 
如果编译过程中出现找不到头文件bzlib.h，需要先安装：
sudo apt-get install libbz2-dev

##### protobuf的安装：

下载： https://github.com/google/protobuf/releases <br>

编译：<br>
./configuare <br>
make <br>
sudo make install <br>
安装后头文件位置： /usr/local/include/google/protobuf/ <br>
库文件位置：  /usr/local/lib/ <br>

编写协议文件：<br>
vim MHello.proto，内容如下：<br>
message MHello <br>
{ <br>
        required string str = 1; <br>
        required int32 number = 2; <br>
} <br>
编译协议文件：<br>
protoc -I=. --cpp_out=.  MHello.proto <br>

使用：<br>
#include "MHello.pb.h" <br>

序列化：<br>

```
MHello he; <br>
he.set_str("hello"); <br>
he.set_number(1); <br>
std::string str; <br>
he.SerializeToString(&str); <br>
```

反序列化：

```
MHello he2; <br>
he2.ParseFromString(str); <br>
```

大概性能：<br>
protobuf    200W次序列及反序列/S <br>
msgpack     50W次序列及反序列/S <br>
jsoncpp     5W次序列及反序列/S <br>

##### jsoncpp的安装：

下载jsoncpp：http://sourceforge.net/projects/jsoncpp <br>
文档：http://json.org/json-zh.html <br>

由于jsoncpp需要使用scons编译，需要下载scons（scons采用python编写）<br>
http://www.scons.org <br>

编译：<br>
python /home/kdjie/scons-2.3.1/script/scons platform=linux-gcc <br>

在jsoncpp-src-0.5.0/libs/linux-gcc-4.3.3目录下生成 <br>
libjson_linux-gcc-4.3.3_libmt.a <br>
libjson_linux-gcc-4.3.3_libmt.so <br>

一些旧版本的编译方法：<br>
export MYSCONS=/home/kdjie/scons-2.1.0 <br>
export SCONS_LIB_DIR=$MYSCONS/engine <br>
python $MYSCONS/script/scons platform=linux-gcc <br>

安装：<br>
sudo cp include/json /usr/local/include/ -r <br>
sudo cp libs/linux-gcc-4.3.3/* /usr/local/lib/ <br>

----------------------------------------------------------
### 使用介绍：

