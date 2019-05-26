//============================================================================
// Name        : Logger.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <syslog.h>
#include <time.h>
#include <string>

namespace evwork
{
	// 日志输出级别定义
	// 保持与<sys/syslog.h>级别定义一致
	enum LogLevel
	{
		Error = 3,		// LOG_ERR
		Warn = 4,		// LOG_WARNING
		Notice = 5,		// LOG_NOTICE
		Info = 6,		// LOG_INFO
		Debug = 7		// LOG_DEBUG
	};

	inline std::string LevelToString(int iLevel)
	{
		switch (iLevel)
		{
			case Error: return "error";
			case Warn: return "warn";
			case Notice: return "notice";
			case Info: return "info";
			case Debug: return "debug";
		}
		
		char szNumber[16] = {0};
		snprintf(szNumber, sizeof(szNumber), "level:%d", iLevel);
		return std::string(szNumber);
	}

	inline std::string getCurrentTime()
	{
			time_t tmNow = time(NULL);
			struct tm* pLT = localtime(&tmNow);
			char szTime[128] = {0};
			snprintf(szTime, sizeof(szTime), "%d-%02d-%02d %02d:%02d:%02d",
					pLT->tm_year + 1900,
					pLT->tm_mon + 1,
					pLT->tm_mday,
					pLT->tm_hour,
					pLT->tm_min,
					pLT->tm_sec);
			return std::string(szTime);
	}

	class ILogReport
	{
	public:
		virtual ~ILogReport() {}

		virtual void log(int iLevel, const char* szFormat, ...) = 0;
	};

	class ILogReportAware
	{
	protected:
		ILogReport* m_pLogReport;
	public:
		ILogReportAware() : m_pLogReport(NULL) {}
		virtual ~ILogReportAware() {}

		void setLogReport(ILogReport* p) { m_pLogReport = p; }
		ILogReport* getLogReport() { return m_pLogReport; }
	};

	class CSyslogReport
		: public ILogReport
	{
	public:
		CSyslogReport()
		: m_iLevelUp(Debug)
		, m_bTerminal(false)
		{
			openlog(NULL, LOG_PID, LOG_USER);
		}
		virtual ~CSyslogReport()
		{
			closelog();
		}

		void setLevelUp(int iLevel)
		{
			m_iLevelUp = iLevel;
		}
		void setTerminal(bool b)
		{
			m_bTerminal = b;
		}

		virtual void log(int iLevel, const char* szFormat, ...)
		{
			if (iLevel > m_iLevelUp)
				return;
				
			va_list va;
			va_start(va, szFormat);
			vsyslog(iLevel, szFormat, va);
			va_end(va);

			if (m_bTerminal)
			{
				va_list va;
				va_start(va, szFormat);
				printf("%s %s ", getCurrentTime().c_str(), LevelToString(iLevel).c_str());
				vprintf(szFormat, va);
				printf("\n");
				va_end(va);
			}
		}

	private:
		int m_iLevelUp;
		bool m_bTerminal;
	};
}
