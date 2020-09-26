#include "libevwork/EVWork.h"
#include "libevwork/ListenConn.h"

#include "libevwork/TimerHandler.h"
#include "libevwork/jsmfc/FormDef.h"
#include "libevwork/jsmfc/Sender.h"
#include "libevwork/jsmfc/DataHandler.h"
#include "libevwork/jsmfc/MfcAppContext.h"

using namespace evwork;
using namespace js;

// 消息命令字定义
#define MESSAGE_ID_1	1
#define MESSAGE_ID_2	2

class CEchoSender
    : public js::PHClass
{
public:
	DECLARE_JS_FORM_MAP;

	CEchoSender()
	{
		m_timerSender.init(this);
		m_timerSender.start(5000);
	}
	~CEchoSender()
	{
		m_timerSender.stop();
	}

    	// 协议处理
	void onMessage2(Json::Value* pJson, evwork::IConn* pConn);

private:
	bool __SenderFunc()
	{
        std::string strMsg("{\"hello\":\",world!\"}");
        printf("[CEchoSender::%s] send -> %s \n", __FUNCTION__, strMsg.c_str());

        js::Sender sdr(MESSAGE_ID_1, strMsg);
        CEnv::getThreadEnv()->getWriter()->send("127.0.0.1", 1982, sdr.Data(), sdr.Size());
        CEnv::getThreadEnv()->getWriter()->flush();

        return true;
	}

	TimerHandler<CEchoSender, &CEchoSender::__SenderFunc> m_timerSender;
};

BEGIN_JS_FORM_MAP(CEchoSender)
	ON_JS_REQUEST_CONN(MESSAGE_ID_2, &CEchoSender::onMessage2)
END_JS_FORM_MAP()

// 协议处理
void CEchoSender::onMessage2(Json::Value* pJson, evwork::IConn* pConn)
{
	//LOG(Info, "[CEchoSender::%s] conn:[%d] call...", __FUNCTION__, pConn->getcid());
	printf("[CEchoSender::%s] conn:[%d] call... \n", __FUNCTION__, pConn->getcid());

	// 这里可以分析Json格式，取出数据做相应处理

	// 这里简单打印Json内容
	std::string strJsonText;
	{
		Json::FastWriter writer;
		strJsonText = writer.write(*pJson);

		//LOG(Debug, "[CEchoSender::%s] onMessage2:[%s]", __FUNCTION__, strJsonText.c_str());
		printf("[CEchoSender::%s] echo message2:[%s]", __FUNCTION__, strJsonText.c_str());
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

	CEnv::getThreadEnv()->getEVParam().uConnTimeout = 300;

	LP.init();

	//-------------------------------------------------------------------------
	// 应用程序初使化

	js::CDataHandler __DE;
	__DE.setPacketLimit(16*1024); 
	CEnv::getThreadEnv()->setDataEvent(&__DE);

	js::CMfcAppContext __MFC;
	__DE.setAppContext(&__MFC);

	CEchoSender __obj;
	__MFC.addEntry(CEchoSender::getFormEntries(), &__obj);

	//-------------------------------------------------------------------------
	// 启动事件循环

	LP.runLoop();

	return 0;
}
