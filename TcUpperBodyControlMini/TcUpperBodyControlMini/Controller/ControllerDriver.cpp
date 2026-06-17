///////////////////////////////////////////////////////////////////////////////
// ControllerDriver.cpp
#include "TcPch.h"
#pragma hdrstop

#include "ControllerDriver.h"
#include "ControllerClassFactory.h"

DECLARE_GENERIC_DEVICE(CONTROLLERDRV)

IOSTATUS CControllerDriver::OnLoad( )
{
	TRACE(_T("CObjClassFactory::OnLoad()\n") );
	m_pObjClassFactory = new CControllerClassFactory();

	return IOSTATUS_SUCCESS;
}

VOID CControllerDriver::OnUnLoad( )
{
	delete m_pObjClassFactory;
}

unsigned long _cdecl CControllerDriver::CONTROLLERDRV_GetVersion( )
{
	return( (CONTROLLERDRV_Major << 8) | CONTROLLERDRV_Minor );
}

