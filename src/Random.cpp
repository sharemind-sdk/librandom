/*
* This file is a part of the Sharemind framework.
* 
* Copyright (C) Dan Bogdanov, 2006-2007
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
*/


//BEGIN_USER_SECTION_BEFORE_MASTER_INCLUDE
#include <cstdlib>
#include <ctime>
#include <GetTime.h>
//END_USER_SECTION_BEFORE_MASTER_INCLUDE


#include "Sharemind.h"

//BEGIN_USER_SECTION_AFTER_MASTER_INCLUDE

//END_USER_SECTION_AFTER_MASTER_INCLUDE


Random::Random()

{//BEGIN_7fa46f81df9a4f2ecc553906ec25a24a
	srand ((unsigned int)RakNet::GetTimeNS ());
}//END_7fa46f81df9a4f2ecc553906ec25a24a



Random::~Random()
{//BEGIN_44e898fb061d72f71d69548524b41cf5

}//END_44e898fb061d72f71d69548524b41cf5


val_t Random::Generate()
{//BEGIN_1d6a1a40d23a55298ba2b9a62b526e11
//	return 1;
	return rand ();
}//END_1d6a1a40d23a55298ba2b9a62b526e11


//BEGIN_USER_SECTION_AFTER_GENERATED_CODE

//END_USER_SECTION_AFTER_GENERATED_CODE
