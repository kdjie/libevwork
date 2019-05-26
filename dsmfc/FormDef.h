//============================================================================
// Name        : FormDef.h
// Author      : kdjie
// Version     : 1.0
// Copyright   : @2015
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include "libevwork/EVComm.h"
#include "Request.h"

#include <memory>

namespace ds
{

	class IAppContext
	{
	public:
		virtual ~IAppContext() {};

		virtual void RequestDispatch(Request& request, evwork::IConn* pConn) = 0;
	};
	class IAppContextAware
	{
	protected:
		IAppContext* m_pAppContext;
	public:
		IAppContextAware() : m_pAppContext(NULL) {}
		virtual ~IAppContextAware() {}

		void setAppContext(IAppContext* pAppContext) { m_pAppContext = pAppContext; }
		IAppContext* getAppContext() { return m_pAppContext; }
	};


	// 类MFC消息映射实现

	struct PHClass{};

	typedef void (PHClass::*TargetFunc)();

	// 采用union用法实现不同的形参的函数转换
	union TargetProc
	{
		TargetFunc mf_oo;
		void (PHClass::*mf_vv)();
		void (PHClass::*mf_vc)(void*);
		void (PHClass::*mf_vcc)(void*, evwork::IConn*);
	};

	// 解包基类
	class IFromHandle
	{
	public:
		virtual ~IFromHandle() {}
		virtual void* handlePacket(ds::Unpack& up) = 0;
		virtual void destroyFrom(void* p) = 0;
	};

	// 解包类模板
	template<typename TForm>
	class FormHandleT
		: public IFromHandle
	{
	public:
		virtual void* handlePacket(ds::Unpack& up)
		{
			std::auto_ptr<TForm> obj(new TForm);
			up >> *obj;
			return obj.release();
		}
		virtual void destroyFrom(void* p)
		{
			if (p)
			{
				delete (TForm*)p;
			}
		}
	};

	// 消息映射函数类型
	enum FormProcType
	{
		fpt_vv,			// void ::()
		fpt_vc,			// void ::(Cmd*)
		fpt_vcc,		// void ::(Cmd *, Conn*)
	};

	// 消息映射记录定义
	struct FormEntry
	{
		uint32_t m_uUri;
		//IFromHandle* m_pFormHandle;
		std::auto_ptr<IFromHandle> m_pFormHandle;

		uint32_t m_uType;
		TargetFunc m_pFunc;
		PHClass* m_pObj;
	};

	// 消息映射宏

	#define DECLARE_DS_FORM_MAP \
		static ds::FormEntry *getFormEntries(); \
		static ds::FormEntry formEntries[];

	#define BEGIN_DS_FORM_MAP(theClass) \
		ds::FormEntry* theClass::getFormEntries() { return theClass::formEntries; } \
		ds::FormEntry theClass::formEntries[] = {

	#define END_DS_FORM_MAP() \
		{0, std::auto_ptr<ds::IFromHandle>(NULL), ds::fpt_vv, NULL, NULL} \
		};

	#define ON_DS_REQUEST(hClass, fp) \
		{ hClass::uri, std::auto_ptr<ds::IFromHandle>(new ds::FormHandleT<hClass>()), ds::fpt_vc, (ds::TargetFunc)(static_cast<void (ds::PHClass::*)(hClass *)>(fp)), NULL},

	#define ON_DS_REQUEST_CONN(hClass, fp) \
		{ hClass::uri, std::auto_ptr<ds::IFromHandle>(new ds::FormHandleT<hClass>()), ds::fpt_vcc, (ds::TargetFunc)(static_cast<void (ds::PHClass::*)(hClass *, evwork::IConn*)>(fp)), NULL},
}	 
