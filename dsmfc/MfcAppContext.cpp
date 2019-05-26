//============================================================================
// Name        : MfcAppContext.cpp
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#include "MfcAppContext.h"

#include "libevwork/EVWork.h"

using namespace evwork;
using namespace ds;

CMfcAppContext::CMfcAppContext()
{
}

CMfcAppContext::~CMfcAppContext()
{
}

void CMfcAppContext::addEntry(FormEntry* pEntry, void* pTarget)
{
	for (int i = 0; pEntry[i].m_uUri != 0; ++i)
	{
		if (m_mapEntry.find(pEntry[i].m_uUri) != m_mapEntry.end())
		{
			LOG(Error, "[ds::CMfcAppContext::%s] dupicate form entry, uri:%s", __FUNCTION__, uri2str(pEntry[i].m_uUri));
			assert(false);
		}

		pEntry[i].m_pObj = (PHClass*)pTarget;
		m_mapEntry[pEntry[i].m_uUri] = &pEntry[i];
	}
}

void CMfcAppContext::RequestDispatch(Request& request, evwork::IConn* pConn)
{
	ENTRY_MAP_t::iterator iter = m_mapEntry.find(request.getUri());
	if (iter == m_mapEntry.end())
	{
		DefaultDispatch(request, pConn);
		return;
	}

	FormEntry* pEntry = iter->second;

	void* pPacket = pEntry->m_pFormHandle.get()->handlePacket(request.getUnpack());

	TargetProc prc;
	prc.mf_oo = pEntry->m_pFunc;

	switch (pEntry->m_uType)
	{
	case fpt_vv:
		(pEntry->m_pObj->*prc.mf_vv)();
		break;
	case fpt_vc:
		(pEntry->m_pObj->*prc.mf_vc)(pPacket);
		break;
	case fpt_vcc:
		(pEntry->m_pObj->*prc.mf_vcc)(pPacket, pConn);
		break;
	default:
		LOG(Warn, "[ds::CMfcAppContext::%s] request entry: %s not compare fpt type", __FUNCTION__, uri2str(request.getUri()));
	}

	pEntry->m_pFormHandle.get()->destroyFrom(pPacket);
}

void CMfcAppContext::DefaultDispatch(Request& request, evwork::IConn* pConn)
{
	LOG(Warn, "[ds::CMfcAppContext::%s] not find request entry: %s", __FUNCTION__, uri2str(request.getUri()));
}
