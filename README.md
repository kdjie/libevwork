libevwork 轻量级C++网络引擎库
==========================================================

### 项目说明：
之所以想写这个库，源于2015年，当时我在长沙一家手游戏公司做后端服务。公司原有的服务器代码，虽然基于知名的开源事件库libev，但是所有的底层函数和业务逻辑全部耦合在一个大文件里，动辄几千上万行的代码，基本上没有明显的层次关系，开发质量经常得不到保障。鉴于此，我仍然基于libev，并结合以前我对网络引擎的理解，编写了这个库，应用于团队后续的项目开发中，目前已有3年多时间。

libevwork的名称也有此而来，基于libev底层事件库，实现更高一层的连接管理和收发缓冲管理，并且提供多种应用层协议支持。它整体的工作流程是这样的：<br>
libev事件 -> 连接管理 -> 收发包缓冲 -> 应用层协议解包 -> 消息队列 -> 消息分派 -> 消息处理。<br>
用好libevwork，使用者不需要过多的关注底层（libev及至以上各流程）的实现细节，只需要简单定义好消息结构，并注册好消息回调函数既可，并且提供友好的MFC编程风格。

----------------------------------------------------------
### 编译说明：

编译libevwork前，需要安装以下依赖库：
1. libev <br>
2. libboost-dev <br>
3. protobuf <br> 
4. jsoncpp <br>

其中，protobuf和jsoncpp是可选的，如果在你的应用程序中用不着protobuf和json协议，可以暂时跳过下面protobuf和jsoncpp库的安装。不过，在编译libevwork的时候，需要修改下Makefile文件，这个后面会讲。<br>

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
./bootstrap.sh <br>
注：也可以使用前缀选项指定安装目录，如 <br>
./bootstrap.sh --prefix=/home/kdjie/boost_1_43_0/boost_install，不指定prefix默认安装到/usr/local/include和/usr/local/lib。<br>
 
2. 编译boost，执行：<br>
./bjam <br>
或 <br>
./bjam -a 重新编译 <br>
默认生成的中间文件在./bin.v2目录，所有的库会被收集在stage/lib目录下。<br>

如果编译过程中出现找不到头文件bzlib.h，需要先安装：<br>
sudo apt-get install libbz2-dev <br>

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
先解压jsoncpp和scon压缩包，解后进入jsoncpp，如：<br>
cd /home/kdjie/jsoncpp-src-0.6.0-rc2 <br>
python /home/kdjie/scons-2.3.1/script/scons platform=linux-gcc <br>

在jsoncpp-src-0.6.0/libs/linux-gcc-4.8.8（随gcc版本不同）目录下生成 <br>
libjson_linux-gcc-4.8.5_libmt.a <br>
libjson_linux-gcc-4.8.5_libmt.so <br>

一些旧版本的编译方法：<br>
export MYSCONS=/home/kdjie/scons-2.1.0 <br>
export SCONS_LIB_DIR=$MYSCONS/engine <br>
python $MYSCONS/script/scons platform=linux-gcc <br>

安装：<br>
sudo cp include/json /usr/local/include/ -r <br>
sudo cp libs/linux-gcc-4.8.5/* /usr/local/lib/ <br>
创建符号链接：<br>
cd /usr/local/lib <br>
sudo ln -s libjson_linux-gcc-4.8.5_libmt.so libjsoncpp.so <br>
sudo ln -s libjson_linux-gcc-4.8.5_libmt.a libjsoncpp.a <br>
sudo ldconfig <br>

##### 编译libevwork：

在以上库都安装好之后，进入到libevwork目录下，执行如下命令： <br>
make 或 make all  <br>
即可生成libevwork.a。 <br>

如果前面你跳过了jsoncpp和protobuf的安装，那么你需要修改Makefile，找到这一行： <br>
SRC_FILES   = $(wildcard *.cpp jsmfc/*.cpp pbmfc/*.cpp dsmfc/*.cpp) <br>
修改为： <br>
SRC_FILES   = $(wildcard *.cpp dsmfc/*.cpp) <br>
然后重新make即可。 <br>

----------------------------------------------------------
### 实现一个简单的回显服务：

1. 编写服务器代码，如SimpleEcho.cpp，输入以下内容： <br>

```
#include "libevwork/EVWork.h"

using namespace evwork;

class CDataEvent
	: public IDataEvent
{
public:
	virtual int onData(IConn* pConn, const char* pData, size_t uSize)
	{
		pConn->sendBin(pData, uSize);
		return uSize;
	}
};

int main(int argc, char* argv[])
{
	//-------------------------------------------------------------------------
	// libevwork初使化

	signal(SIGPIPE, SIG_IGN);

	CSyslogReport LG;
	CEVLoop LP;
	CConnManager CM;
	CWriter WR;

	CEnv::getThreadEnv()->setLogger(&LG);
	CEnv::getThreadEnv()->setEVLoop(&LP);
	CEnv::getThreadEnv()->setLinkEvent(&CM);
	CEnv::getThreadEnv()->setConnManager(&CM);
	CEnv::getThreadEnv()->setWriter(&WR);

	CEnv::getThreadEnv()->getEVParam().uConnTimeout = 300;

	LP.init();

	//-------------------------------------------------------------------------
	// 应用程序初使化

	CDataEvent DE;
	CEnv::getThreadEnv()->setDataEvent(&DE);

	CListenConn listenConn(1982);

	//-------------------------------------------------------------------------
	// 启动事件循环

	LP.runLoop();

	return 0;
}
```

编译，执行命令：<br>
g++ -o SimpleEcho SimpleEcho.cpp -I.. -L../libevwork -levwork -lev -lboost_thread <br>
这里 -I.. -L../libevwork 表示设置头文件和库文件的搜索路径。<br>

测试，启动服务：<br>
[kdjie@localhost test.d]$ ./SimpleEcho <br>
启动客户端：<br>
[kdjie@localhost ~]$ telnet 0 1982 <br>
Trying 0.0.0.0... <br>
Connected to 0. <br>
Escape character is '^]'. <br>
hello,world <br>
hello,world <br>

2. 利用Json协议，重新实现简单服务回显，如SimpleJsonEcho.cpp，输入以下内容：<br> 

```
#include "libevwork/EVWork.h" 

#include "libevwork/jsmfc/FormDef.h"
#include "libevwork/jsmfc/Sender.h"
#include "libevwork/jsmfc/DataHandler.h"
#include "libevwork/jsmfc/MfcAppContext.h"

using namespace evwork;
using namespace js;

// 消息命令字定义
#define MESSAGE_ID_1	1
#define MESSAGE_ID_2	2

class CMessageLogic
	: public js::PHClass
{
public:
	DECLARE_JS_FORM_MAP;

	// 协议处理
	void onMessage1(Json::Value* pJson, evwork::IConn* pConn);
};

BEGIN_JS_FORM_MAP(CMessageLogic)
	ON_JS_REQUEST_CONN(MESSAGE_ID_1, &CMessageLogic::onMessage1)
END_JS_FORM_MAP()

// 协议处理
void CMessageLogic::onMessage1(Json::Value* pJson, evwork::IConn* pConn)
{
	//LOG(Info, "[CMessageLogic::%s] conn:[%d] call...", __FUNCTION__, pConn->getcid());
	printf("[CMessageLogic::%s] conn:[%d] call...", __FUNCTION__, pConn->getcid());

	// 这里可以分析Json格式，取出数据做相应处理

	// 这里简单打印Json内容
	std::string strJsonText;
	{
		Json::FastWriter writer;
		strJsonText = writer.write(*pJson);

		//LOG(Debug, "[CMessageLogic::%s] message1:[%s]", __FUNCTION__, strJsonText.c_str());
		printf("[CMessageLogic::%s] message1:[%s]", __FUNCTION__, strJsonText.c_str());
	}

	// 发回响应，这里简单将原包发回
	{
		js::Sender sdr(MESSAGE_ID_2, strJsonText);
		pConn->sendBin(sdr.Data(), sdr.Size());
	}
}

int main(int argc, char* argv[])
{
	//-------------------------------------------------------------------------
	// libevwork初使化

	signal(SIGPIPE, SIG_IGN);

	CSyslogReport LG;
	CEVLoop LP;
	CConnManager CM;
	CWriter WR;

	CEnv::getThreadEnv()->setLogger(&LG);
	CEnv::getThreadEnv()->setEVLoop(&LP);
	CEnv::getThreadEnv()->setLinkEvent(&CM);
	CEnv::getThreadEnv()->setConnManager(&CM);
	CEnv::getThreadEnv()->setWriter(&WR);

	LP.init();

	//-------------------------------------------------------------------------
	// 应用程序初使化

	// 设置连接超时，单位S
	CEnv::getThreadEnv()->getEVParam().uConnTimeout = 300; 

	// 定义数据包处理器
	js::CDataHandler __DE;

	// 数据包处理器限制请求包的最大长度，单位字节
	__DE.setPacketLimit(16*1024); 

	// 关联数据包处理器
	CEnv::getThreadEnv()->setDataEvent(&__DE);

	// 设置MFC对象
	js::CMfcAppContext __MFC;

	// 将MFC对象关联到数据包处理器
	__DE.setAppContext(&__MFC);

	// 定义逻辑主对象
	CMessageLogic __logic;

	// 将主对象的消息映射表装载到MFC对象
	// 同样的，可以定义别的逻辑对象，也装载到MFC对象中
	__MFC.addEntry(CMessageLogic::getFormEntries(), &__logic);

	// 创建服务套接口对象
	CListenConn __listenConn(1982); 

	//-------------------------------------------------------------------------
	// 启动事件循环

	LP.runLoop();

	return 0;
}
```

编译，执行命令：<br>
g++ -o SimpleJsonEcho SimpleJsonEcho.cpp -I.. -L../libevwork -levwork -lev -lboost_thread -ljsoncpp <br>

测试，启动服务：<br>
[kdjie@localhost test.d]$ ./SimpleJsonEcho <br>

客户端见samples/SimpleJsonEcho下的源码文件JsonClient.cpp，编译：<br>
g++ -o JsonClient JsonClient.cpp -I.. -L../libevwork -levwork -lev -lboost_thread -ljsoncpp <br>
启动：<br>
[kdjie@localhost test.d]$ ./JsonClient <br>
[CEchoSender::__SenderFunc] send -> {"hello":",world!"} <br>
[CEchoSender::onMessage2] conn:[1] call... <br>
[CEchoSender::onMessage2] echo message2:[{"hello":",world!"} <br>
][CEchoSender::__SenderFunc] send -> {"hello":",world!"}  <br>
[CEchoSender::onMessage2] conn:[1] call... <br>
[CEchoSender::onMessage2] echo message2:[{"hello":",world!"} <br>

----------------------------------------------------------
### libevwork的设计思想：

在前面的例子中，我们用到了以下类，列举如下：<br>
CSyslogReport  -- Syslog日志输出类，通常在进程启动时，首先初使化这个对象。<br>
CEVLoop  -- 事件循环类，也是对libev的封装，在这里添加/删除事件对象，并且管理事件循环，相当于整个框架的发动机。<br>
CConnManager  -- 连接管理器，负责管理接入和连出的TCP连接。<br>
CWriter  -- 发包器，使用它可以向指定的IP:Port建立连接，并发送数据。<br>
CDataEvent  -- 数据事件接收器，TCP连接收到传输流后，交给它对数据流按具体的传输协议进行消息拆分。它是IDataEvent接口的实现。<br>
CListenConn  -- 服务端监听端口工作类，通过它创建服务器端口。<br>
js::CDataHandler  -- 也是IDataEvent接口的实现，负责对Json数据流进行消息拆分。<br>
js::CMfcAppContext  -- Vistual MFC风格的消息分派器，负责接受具体的应用消息类型回调注册，并进行消息分派。<br>
IConn*  -- 所有的TCP连接基类，通过它可以直接向网络发包。它的客户端实现是CClientConn。<br>

它们的组织关系是这样的：<br>
```
              ——————应用层回调 
               |         ^ 
               |         | 
               |   js::CMfcAppContext 
               |         ^ 
               V         | 
            CWriter  js::CDataHandler 
               |         ^ 
               V         | 
         CConnManager    | 
           ^      ^      |
          /        \     |
     CListenConn  CClientConn
          ^        ^
           \      /
           CEVLoop
              ^
              |
            libev
```

libevwork的设计思想，就是尽可能的减小代码，还能适配大部分TCP服务器开发的场景。在上面这个类组织关系图中，实际上它们之间是以接口方式互相组织的，接口的对应关系如下：<br>
IConn -> CClientConn <br>
IConnManager -> CConnManager <br>
ILinkEvent -> CConnManager <br>
IDataEvent -> CDataEvent、js::CDataHandler <br>
IWriter -> CWriter <br>
IAppContext -> js::CMfcAppContext <br>
所以，根据业务的需要，只要继承这些接口，并重载实现来满足业务多样化的需求。例如，为了支持多线程，本类库实现了CAsyncWriter和CAsyncDataHandler，它们分别继承自IWriter、IDataEvent。<br>

----------------------------------------------------------
### libevwork代码结构及使用说明：

首先，让我们来看一下libevwork的文件树结构：<br>

```
libevwork
├── EVComm.h
├── EVLoop.cpp
├── EVWork.cpp
├── EVWork.h
├── ExceptionErrno.h
├── FuncHelper.h
├── AsyncWriter.cpp
├── AsyncWriter.h
├── Buffer.h
├── ClientConn.cpp
├── ClientConn.h
├── ConnManager.cpp
├── ConnManager.h
├── ListenConn.cpp
├── ListenConn.h
├── Logger.h
├── TimerHandler.h
├── Writer.cpp
├── Writer.h
├── Makefile
├── README.md
├── dsmfc
│?? ├── AsyncDataHandler.cpp
│?? ├── AsyncDataHandler.h
│?? ├── DataHandler.cpp
│?? ├── DataHandler.h
│?? ├── ds
│?? │?? ├── dsbuffer.h
│?? │?? ├── dspacket.h
│?? │?? └── dstypes.h
│?? ├── FormDef.h
│?? ├── MfcAppContext.cpp
│?? ├── MfcAppContext.h
│?? ├── Request.h
│?? └── Sender.h
├── jsmfc
│?? ├── AsyncDataHandler.cpp
│?? ├── AsyncDataHandler.h
│?? ├── DataHandler.cpp
│?? ├── DataHandler.h
│?? ├── FormDef.h
│?? ├── MfcAppContext.cpp
│?? ├── MfcAppContext.h
│?? ├── Request.h
│?? └── Sender.h
├── pbmfc
│?? ├── AsyncDataHandler.cpp
│?? ├── AsyncDataHandler.h
│?? ├── DataHandler.cpp
│?? ├── DataHandler.h
│?? ├── FormDef.h
│?? ├── MfcAppContext.cpp
│?? ├── MfcAppContext.h
│?? ├── Request.h
│?? └── Sender.h
```

可以看到整体是一个以两层目录结构的树，其中许多文件都可以顾名思义。第一层为通用实现，提供基础连接管理和收发包的支持。第二层包启dsmfc、jsmfc、pbmfc三个目录，分别对三种流传输协议提供压解包和消息分派支持。<br>

在介绍使用方法之前，先看几个文件：<br>

EVComm.h >>>>>>>>>>> <br>

```
namespace evwork
{
	// 连接对象
	struct IConn
	{
	public:
		IConn() : m_fd(-1), m_cid(0) {}
		virtual ~IConn() {}

		void setcid(uint32_t cid) { m_cid = cid; }
		uint32_t getcid() { return m_cid; }

		virtual void getPeerInfo(std::string& strPeerIp, uint16_t& uPeerPort16) = 0;
		virtual bool sendBin(const char* pData, size_t uSize) = 0;

	protected:
		int m_fd;
		uint32_t m_cid;
	};

	// 连接事件
	struct ILinkEvent
	{
	public:
		virtual ~ILinkEvent() {}

		virtual void onConnected(IConn* pConn) = 0;
		virtual void onClose(IConn* pConn) = 0;
	};

	// 数据事件
	struct IDataEvent
	{
	public:
		virtual ~IDataEvent() {}

		virtual int onData(IConn* pConn, const char* pData, size_t uSize) = 0;
	};

	// 事件句柄
	struct IHandle
	{
	public:
		IHandle() : m_fd(-1), m_ev(0) { m_evio.data = this; }
		virtual ~IHandle() {}

		void setFd(int fd) { m_fd = fd; }
		int getFd() { return m_fd; }

		void setEv(int ev) { m_ev = ev; }
		int getEv() { return m_ev; }

		ev_io& getEvIo() { return m_evio; }

		virtual void cbEvent(int revents) = 0;

		static void evCallBack(struct ev_loop *loop, struct ev_io *w, int revents)
		{
			IHandle* pThis = (IHandle*)w->data;

			pThis->cbEvent(revents);
		}

	protected:
		int m_fd;
		int m_ev;
		ev_io m_evio;
	};

	// 模板事件句柄
	template <typename T, void (T::*fn)(int revents)>
	class THandle
		: public IHandle
	{
	public:
		THandle(T* p) : m_pObj(p) {}
		virtual ~THandle() {}

		void cbEvent(int revents)
		{
			(m_pObj->*fn)(revents);
		}

	protected:
		T* m_pObj;
	};

	// EVLook事件循环对象
	class CEVLoop
	{
	public:
		CEVLoop();
		virtual ~CEVLoop();

		bool init();
		void destroy();

		void runLoop();
		void breakLoop();

		void setHandle(IHandle* p);
		void delHandle(IHandle* p);

		struct ev_loop* getEvLoop();

	private:
		struct ev_loop* m_pEVLoop;

		std::set<IHandle*> m_setHandle;
	};

	// TCP发包接口
	struct IWriter
	{
	public:
		virtual ~IWriter() {}

		virtual void send(const std::string& strIp, uint16_t uPort, const char* pData, size_t uSize) = 0;
		virtual void flush() = 0;
	};

	// TCP连接管理
	struct IConnManager
	{
	public:
		virtual ~IConnManager() {}

		virtual IConn* getConnById(uint32_t uConnId) = 0;
		virtual IConn* getConnByIpPort(const std::string& strIp, uint16_t uPort) = 0;
	};
}
```

这个文件定义了IConn、ILinkEvent、IDataEvent、IHandle、CEVLoop、IWriter、IConnManager等基础的接口及相应的方法。<br>

EVWork.h >>>>>>>>>>> <br>

```
namespace evwork
{
	struct SEVParam
	{
		uint32_t uConnTimeout;

		SEVParam()
		{
			uConnTimeout = (uint32_t)-1;
		}
	};

	class CThreadEnv
	{
	public:
		CThreadEnv();
		virtual ~CThreadEnv();

		void setEVLoop(CEVLoop* p);
		CEVLoop* getEVLoop();

		void setLogger(ILogReport* p);
		ILogReport* getLogger();

		void setLinkEvent(ILinkEvent* p);
		ILinkEvent* getLinkEvent();

		void setDataEvent(IDataEvent* p);
		IDataEvent* getDataEvent();

		void setWriter(IWriter* p);
		IWriter* getWriter();

		void setConnManager(IConnManager* p);
		IConnManager* getConnManager();

		SEVParam& getEVParam();

	private:
		CEVLoop* m_pEVLoop;
		ILogReport* m_pLogger;
		ILinkEvent* m_pLinkEvent;
		IDataEvent* m_pDataEvent;
		IWriter* m_pWriter;
		IConnManager* m_pConnManager;
		SEVParam m_evParam;
	};

	class CEnv
	{
	public:
		static CThreadEnv* getThreadEnv();

	private:
		static boost::thread_specific_ptr<CThreadEnv> m_tssEnv;
	};

	#define LOG(l,f,...) CEnv::getThreadEnv()->getLogger()->log(l, f, ##__VA_ARGS__)

}
```

这个文件定义了CThreadEnv结构，它包含了全部基础对象的指针，我们可以把它看成一个线程内部的黑盒子，通过初使化时为黑盒子设置基础对象的指针，它们可以有效地组织成一个整体。文件同时还定义了CEnv，通过调用它的getThreadEnv()方法，可以返回调用线程的CThreadEnv结构，对于单进程（单反应器）结构，我们通常只在主线程调用CEnv::getThreadEnv()方法，如果需要创建多反应器模型，我们可以在每个线程内调用CEnv::getThreadEnv()创建多个CThreadEnv结构，并且为它们分别设置相应的基础对象。<br>

现在可以明白这段代码的含义了吗？<br>

```
//-------------------------------------------------------------------------
	// libevwork初使化

	signal(SIGPIPE, SIG_IGN);

	CSyslogReport LG;
	CEVLoop LP;
	CConnManager CM;
	CWriter WR;

	CEnv::getThreadEnv()->setLogger(&LG);
	CEnv::getThreadEnv()->setEVLoop(&LP);
	CEnv::getThreadEnv()->setLinkEvent(&CM);
	CEnv::getThreadEnv()->setConnManager(&CM);
	CEnv::getThreadEnv()->setWriter(&WR);

	CEnv::getThreadEnv()->getEVParam().uConnTimeout = 300;

	LP.init();

	//-------------------------------------------------------------------------
	// 应用程序初使化

	CDataEvent DE;
	CEnv::getThreadEnv()->setDataEvent(&DE);

	CListenConn listenConn(1982);

	//-------------------------------------------------------------------------
	// 启动事件循环

	LP.runLoop();
```

----------------------------------------------------------
### 一个复杂的例子：

请参考另一款开源工具：极简轻量级可防CC反向代理 <br>
地址：https://github.com/kdjie/portmap 