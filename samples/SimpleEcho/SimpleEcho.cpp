#include "libevwork/EVWork.h"
#include "libevwork/ListenConn.h"

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
