/* ****************************************************************************

 * eID Middleware Project.
 * Copyright (C) 2009-2010 FedICT.
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

/*
 * Integration test for the PKCS#11 library.
 * Required interaction: none.
 */

#include <stdio.h>
#include <MainTest.h>
#include "logtest.h"

#define NUMBER_OF_TESTS 100

int main() {
	int i=0;
	int testCounter = 0;
	char* testDescription[NUMBER_OF_TESTS];
	testRet result[NUMBER_OF_TESTS];

	initLog();

	while (i<NUMBER_OF_TESTS)
	{
		testDescription[i] = NULL;
		i++;
	}
	testDescription[testCounter] = "Test multiple finalize/initialize sessions in multiple threads with different args";
	result[testCounter] = test_finalize_initialize();
	testCounter++;
	testDescription[testCounter] = "Test multiple finalize/initialize sessions in a single thread";
	result[testCounter] = test_finalize_initialize_st();
	testCounter++;
	testDescription[testCounter] = "Test initialize when bad parameters are supplied";
	result[testCounter] = test_initialize_ownmutex();
	testCounter++;
	result[testCounter] = test_initialize_preserved();
	testCounter++;
	result[testCounter] = test_finalize_preserved();
	testCounter++;
	testDescription[testCounter] = "Test C_getinfo results in a single thread";
	result[testCounter] = test_getinfo();
	testCounter++;
	testDescription[testCounter] = "Test C_getslotlist in single and two call usage";
	result[testCounter] = test_getslotlist();
	testCounter++;
	testDescription[testCounter] = "Test C_getslotlist when insufficient memory is reserved";
	result[testCounter] = test_getslotlist_multiple_slots();
	testCounter++;
	testDescription[testCounter] = "Tests opening and closing of a session in a single thread";
	result[testCounter] = test_open_close_session();
	testCounter++;
	testDescription[testCounter] = "Shows info on the mechanisms supported by the card";
	result[testCounter] = test_getmechanisms();
	testCounter++;
	testDescription[testCounter] = "tests getting the signature key from the card";
	result[testCounter] = test_getprivatekeys();
	testCounter++;
	testDescription[testCounter] = "tests getting all the objects from the card";
	result[testCounter] = test_getallobjects();
	testCounter++;
	testDescription[testCounter] = "tests signing with the card";
	result[testCounter] = test_sign();
	testCounter++;

	//testlog(LVL_NOLEVEL,"\n\n_______________________________________________\n");
	for (i = 0; i < testCounter; i++)
	{
		if (testDescription[i] != NULL)
		{
			testlog(LVL_NOLEVEL,"\n_______________________________________________\n");
			testlog(LVL_NOLEVEL,"\nTest %d %s \n", i, testDescription[i]);
		}
		if(result[i].pkcs11rv != CKR_OK)
		{
			testlog(LVL_NOLEVEL,"FAILED : Result = 0x%.8x \n", result[i].pkcs11rv);
		}
		else if(result[i].basetestrv == TEST_PASSED)
		{
			testlog(LVL_NOLEVEL,"PASSED\n");
		}
		else if(result[i].basetestrv == TEST_SKIPPED)
		{
			testlog(LVL_NOLEVEL,"SKIPPED\n");
		}
		else
		{	
			testlog(LVL_NOLEVEL,"FAILED : Result = 0x%.8x \n", result[i].pkcs11rv);
		}
		
	}
	
	
	endLog();
	// Wait for user to end this test
	getchar();
}

