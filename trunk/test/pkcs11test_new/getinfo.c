/* ****************************************************************************

* eID Middleware Project.
* Copyright (C) 2008-2010 FedICT.
*
* This is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License version
* 3.0 as published by the Free Software Foundation.
*
* This software is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this software; if not, see
* http://www.gnu.org/licenses/.

**************************************************************************** */
#include "basetest.h"
#include "logtest.h"

testRet test_getinfo() {
	void *handle;						//handle to the pkcs11 library
	CK_FUNCTION_LIST_PTR functions;		// list of the pkcs11 function pointers

	testRet retVal = {CKR_OK,TEST_PASSED};	//return values of this test
	CK_RV frv = CKR_OK;						//return value of last pkcs11 function called

	CK_INFO info;
	CK_SESSION_HANDLE session_handle;
	long slot_count;
	CK_SLOT_ID_PTR slotIds;
	int slotIdx;

	testlog(LVL_DEBUG, "test_getinfo enter\n");
	if (InitializeTest(&handle,&functions))
	{
		frv = (*functions->C_Initialize) (NULL);
		if (ReturnedSuccesfull(frv,&(retVal.pkcs11rv), "C_Initialize", "test_getinfo enter" ))
		{
			frv = (*functions->C_GetInfo) (&info);
			if (ReturnedSuccesfull(frv,&(retVal.pkcs11rv), "C_GetInfo", "test_getinfo enter" ))
			{
				testlog(LVL_INFO,"library version: %d.%d\n", info.libraryVersion.major, info.libraryVersion.minor);
				testlog(LVL_INFO,"PKCS#11 version: %d.%d\n", info.cryptokiVersion.major, info.cryptokiVersion.minor);
				if( (info.cryptokiVersion.major != 2) || (info.cryptokiVersion.minor != 11) )
				{
					retVal.basetestrv = TEST_ERROR;
				}
			}
			frv = (*functions->C_Finalize) (NULL_PTR);
			ReturnedSuccesfull(frv,&(retVal.pkcs11rv), "C_Finalize", "test_getinfo enter" );
		}
		dlclose(handle);
	}
	else
	{
		retVal.basetestrv = TEST_ERROR;
	}

	testlog(LVL_DEBUG, "test_getinfo enter leave\n");
	return retVal;
}

