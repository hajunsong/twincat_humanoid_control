///////////////////////////////////////////////////////////////////////////////
// ControllerDriver.h

#ifndef __CONTROLLERDRIVER_H__
#define __CONTROLLERDRIVER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TcBase.h"

#define CONTROLLERDRV_NAME        "CONTROLLER"
#define CONTROLLERDRV_Major       1
#define CONTROLLERDRV_Minor       0

#define DEVICE_CLASS CControllerDriver

#include "ObjDriver.h"

class CControllerDriver : public CObjDriver
{
public:
	virtual IOSTATUS	OnLoad();
	virtual VOID		OnUnLoad();

	//////////////////////////////////////////////////////
	// VxD-Services exported by this driver
	static unsigned long	_cdecl CONTROLLERDRV_GetVersion();
	//////////////////////////////////////////////////////
	
};

Begin_VxD_Service_Table(CONTROLLERDRV)
	VxD_Service( CONTROLLERDRV_GetVersion )
End_VxD_Service_Table


#endif // ifndef __CONTROLLERDRIVER_H__