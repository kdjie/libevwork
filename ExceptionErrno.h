//============================================================================
// Name        : ExceptionErrno.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <exception>

namespace evwork
{
	class exception_errno 
		: public std::exception
	{
	public:
		exception_errno(const std::string& __arg) { init(errno, __arg); }
		exception_errno(int e, const std::string& __arg) { init(e, __arg); }
		virtual ~exception_errno() throw() {};

		int what_errno() const throw() { return m_errno; }
		const std::string & what_str() const throw() { return m_what; }

		virtual const char* what() const throw() { return m_what.c_str(); }

	private:
		void init(int e, const std::string & arg)
		{
			m_errno = e;
			m_what = arg;

			if (e != 0)
			{
				char szTmp[12] = {0};
				snprintf(szTmp, sizeof(szTmp), "%d", m_errno);

				m_what += " - ";
				m_what += std::string(szTmp) + ": ";
				m_what += strerror(m_errno);
			}
		}

	private:
		int m_errno;
		std::string m_what;
	};
}
