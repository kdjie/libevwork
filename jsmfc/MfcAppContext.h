//============================================================================
// Name        : MfcAppContext.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "FormDef.h"

#include <tr1/unordered_map>

namespace js
{

	class CMfcAppContext
		: public IAppContext
	{
	public:
		CMfcAppContext();
		virtual ~CMfcAppContext();

		virtual void addEntry(FormEntry* pEntry, void* pTarget);

		virtual void RequestDispatch(Request& request, evwork::IConn* pConn);

	protected:
		virtual void DefaultDispatch(Request& request, evwork::IConn* pConn);

	protected:
		typedef std::tr1::unordered_map<JS_CMD_TYPE, FormEntry*> ENTRY_MAP_t;

		ENTRY_MAP_t m_mapEntry;
	};

}
