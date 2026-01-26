#include "asm_mfc.h"


static const _PNH _pfnUninitialized = (_PNH)-1;

//5ddf54
void* __cdecl operator new(size_t nSize)
{
	void* pResult;
	_PNH pfnNewHandler = _pfnUninitialized;

	for (;;)
	{
		pResult = malloc(nSize);

		if (pResult != NULL)
			return pResult;

		if (pfnNewHandler == _pfnUninitialized)
		{
			AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();
			pfnNewHandler = pState->m_pfnNewHandler;
		}
		if (pfnNewHandler == NULL || (*pfnNewHandler)(nSize) == 0)
			break;
	}
	return pResult;
}

//5ddf90
void __cdecl operator delete(void* p)
{
	free(p);
}

//609088
const AFX_DATADEF struct CRuntimeClass CObject::classCObject =
{ "CObject", sizeof(CObject), 0xffff, NULL, NULL, NULL };

//5ddf9b
CRuntimeClass* CObject::GetRuntimeClass() const
{
	return RUNTIME_CLASS(CObject);
}

//401900
CObject::CObject()
{}

//401950
CObject::~CObject()
{}

//401970
void CObject::Serialize(CArchive&)
{}


void* PASCAL CObject::operator new(size_t, void* p)
{
	return p;
}

//401860
void PASCAL CObject::operator delete(void* p)
{
	::operator delete(p);
}

//401880
void* PASCAL CObject::operator new(size_t nSize)
{
	return ::operator new(nSize);
}

//401980
void CObject::AssertValid() const
{
}

//401990
void CObject::Dump(CDumpContext&) const
{
}

//5ddfa1
BOOL CObject::IsKindOf(const CRuntimeClass* pClass) const
{
	ASSERT(this != NULL);
	// it better be in valid memory, at least for CObject size
	ASSERT(AfxIsValidAddress(this, sizeof(CObject)));

	// simple SI case
	CRuntimeClass* pClassThis = GetRuntimeClass();
	return pClassThis->IsDerivedFrom(pClass);
}

//5de019
BOOL CObject::IsSerializable() const
{
	return (GetRuntimeClass()->m_wSchema != 0xffff);
}




CObject* CRuntimeClass::CreateObject()
{
	if (m_pfnCreateObject == NULL)
	{
		TRACE(_T("Error: Trying to create object which is not ")
			_T("DECLARE_DYNCREATE \nor DECLARE_SERIAL: %hs.\n"),
			m_lpszClassName);
		return NULL;
	}

	CObject* pObject = NULL;
	TRY
	{
		pObject = (*m_pfnCreateObject)();
	}
		END_TRY

		return pObject;
}



BOOL CRuntimeClass::IsDerivedFrom(const CRuntimeClass* pBaseClass) const
{
	ASSERT(this != NULL);
	ASSERT(AfxIsValidAddress(this, sizeof(CRuntimeClass), FALSE));
	ASSERT(pBaseClass != NULL);
	ASSERT(AfxIsValidAddress(pBaseClass, sizeof(CRuntimeClass), FALSE));

	// simple SI case
	const CRuntimeClass* pClassThis = this;
	while (pClassThis != NULL)
	{
		if (pClassThis == pBaseClass)
			return TRUE;
		pClassThis = pClassThis->m_pBaseClass;
	}
	return FALSE;       // walked to the top, no match
}






//const CObject* AFX_CDECL AfxDynamicDownCast(CRuntimeClass* pClass, const CObject* pObject)
//{
//	return (const CObject*)AfxDynamicDownCast(pClass, (CObject*)pObject);
//}
//
//CObject* AFX_CDECL AfxDynamicDownCast(CRuntimeClass* pClass, CObject* pObject)
//{
//	if (pObject != NULL && pObject->IsKindOf(pClass))
//		return pObject;
//	else
//		return NULL;
//}
///*
//AFX_CLASSINIT::AFX_CLASSINIT(register CRuntimeClass* pNewClass)
//{
//	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
//	AfxLockGlobals(CRIT_RUNTIMECLASSLIST);
//	pModuleState->m_classList.AddHead(pNewClass);
//	AfxUnlockGlobals(CRIT_RUNTIMECLASSLIST);
//}*/
//

