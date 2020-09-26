//============================================================================
// Name        : MfcAppContext.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#include "FormDef.h"

#include <tr1/unordered_map>

namespace ds
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
		typedef std::tr1::unordered_map<DS_URI_TYPE, FormEntry*> MAP_ENTRY_t;

		MAP_ENTRY_t m_mapEntry;
	};

}
