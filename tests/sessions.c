#include <unix.h>
#include <pkcs11.h>
#include <stdio.h>
#include <stdlib.h>

#include "testlib.h"

CK_RV notify(CK_SESSION_HANDLE handle, CK_NOTIFICATION event, CK_VOID_PTR ptr) {
	return CKR_OK;
}

int sessions(void) {
	CK_RV rv;
	CK_SLOT_ID slot;
	CK_ULONG count=0;
	int i, ret;
	CK_SESSION_HANDLE handle;

	rv = C_Initialize(NULL_PTR);
	check_rv;

	if((ret = find_slot(CK_TRUE, &slot)) != TEST_RV_OK) {
		return ret;
	}

	rv = C_OpenSession(slot, 0, NULL_PTR, notify, &handle);
	assert(ckrv_decode(rv, 2, (CK_RV)CKR_OK, (int)TEST_RV_FAIL, (CK_RV)CKR_SESSION_PARALLEL_NOT_SUPPORTED, (int)TEST_RV_OK) == TEST_RV_OK);

	rv = C_OpenSession(slot, CKF_SERIAL_SESSION, NULL_PTR, notify, &handle);
	check_rv;

	rv = C_CloseSession(handle);
	check_rv;

	rv = C_OpenSession(slot, CKF_SERIAL_SESSION | CKF_RW_SESSION, NULL_PTR, notify, &handle);
	check_rv;

	rv = C_Finalize(NULL_PTR);
	check_rv;

	return TEST_RV_OK;
}
