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
