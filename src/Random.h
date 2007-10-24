/*
* This file is a part of the Sharemind framework.
* 
* Copyright (C) Dan Bogdanov, 2006-2007
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
*/
#ifndef H_e62fce1dbd1b3de95777b710bae3a739_H
#define H_e62fce1dbd1b3de95777b710bae3a739_H



//BEGIN_USER_SECTION_BEFORE_CLASS_DECLARATION

//END_USER_SECTION_BEFORE_CLASS_DECLARATION


/**
A random generator. Current naive implementation uses the rand function in C library.

\note The methods are not static so you must make an instance of it to use it.

\todo A cryptographically secure randomness generator would be nice.

*/
class Random
{
// constructors:
public:
Random();


/**
Cleans up
*/
public:
virtual ~Random();

// members:


//methods:

/**
Generates a random value

\returns a random value
*/
public:
val_t Generate();



//child groups:


//child classes:


};

//BEGIN_USER_SECTION_AFTER_CLASS_DECLARATION

//END_USER_SECTION_AFTER_CLASS_DECLARATION


#endif // H_e62fce1dbd1b3de95777b710bae3a739_H

#ifdef OBJECTS_BUILDER_PROJECT_INLINES
#ifndef H_e62fce1dbd1b3de95777b710bae3a739_INLINES_H
#define H_e62fce1dbd1b3de95777b710bae3a739_INLINES_H

#endif // H_e62fce1dbd1b3de95777b710bae3a739_INLINES_H
#endif //OBJECTS_BUILDER_PROJECT_INLINES
