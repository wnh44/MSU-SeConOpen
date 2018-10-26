# Open SeCon Style Guide
###### Authors: Spencer Barnes, Jake Griesmer
###### Updated: 10/25/2018


This style guide will cover various conventions that will be used for the Mississippi 
State open Secon team. The topics covered in this guide should cover both Arduino C and Python 
code.


> "Self-documenting (or self-describing) source code and user interfaces follow naming 
> conventions and structured programming conventions that enable use of the system without 
> prior specific knowledge." 
> - Wikipedia on Self-documenting Code

The design and commenting of the code should thoroughly explain how the code works and the 
decisions made while writing the code. This is done using effective naming and commenting, but 
also in organizing the code to improve readability. 

Lines of code should be shorter than 70 characters and should never exceed 120 characters. There
should be fewer than 8 consecutive lines of code and should never exceed 15 lines.

___
## Order of Code
The files should follow a consitent ordering of code "sections":
1. Comment block header
2. Included libraries
3. Constants and global variables
4. Declare and define any embedded functions (e.g. timers, interupts)
5. Function prototypes
6. Main function or setup and loop functions
7. Definitions of other functions
<br/>

Classes should follow a structure of:
1. Public, protected, private attributes
2. Constructor
3. Destructor
4. Getters and setters
5. Class methods

___
## Commenting
Comments should be used to describe code in logical chunks. They may be used on a more individual
basis or inline with more complicated or convoluted lines. Comments should be simple complete 
phrases that are typically not whole sentences.

#### Descriptive Comments
These are the comments used to describe code purpose and functionality. Each line should have at
least one descriptive comment to give it context. Descriptive comments should begin with an 
uppercase letter, and multi-line comments should having a hanging indent.

```c++
// Run through the linked list, stop when the item's found
//	here is example of multi lined comment
```

#### Debugging Comments
This distinction is made since debugging comments are made to be
temporary. Often these may end up in final code and cause confusion, or these may mark added 
lines for testing that if left in can cause bugs.

Simple temporary comments should start with your NetID and should be a single line.
`// sb2726 just example debug note`

Whenever code is added for test, it must be encapsulated or inlined with 'DEBUG' and your NetID.

```c++
// DEBUG sb2726
print( "Reached debug point" );
// DEBUG 
``` 

#### Comment Block Header
The comment block header should contain the **name of the file**, the **authors**, the **date of 
creation**, the **date of most recent update**, and a **description** of the contents.

```c++
/********************************************************************
* Name:
* 
* Authors:
* Creation Date:
* Update Date:
* 
* Decription:
********************************************************************/
```
or
```python
#####################################################################
# Name:
# 
# Authors:
# Creation Date:
# Update Date:
# 
# Description:
#####################################################################
```

___
## Variables
All variables must be `UpperCamelCase` and must be preceded by a letter indicating 
their type as `xUpperCamelCase`. *I don't like it much but it is helpful.* Useful unambiguous 
names are bigger time saves than short names; give the variable a name that can be seen out of 
context and understood.

When declaring a pointer, attach the asterisk '*' to the variable type.

```c++
int iClasses = 5;
double adTestScores[] = {90.91, 70.91, 42.4};
double* pdTest = &adTestScores[0];
GradeStats uTestStats = GradeStats( adTestScores );
bool bIsSad = true;
```

#### Constants and Globals
All **constants** must be `CAPITALIZED_WITH_UNDERSCORES`.

All **globals** must be prefixed with 'g\_' as `g_xUpperCamelCase`.

#### Class Attributes
**Non-private attributes** associated with individual object instances must be prefixed with 
'm\_' as `m_xUpperCamelCase`, while **private attributes** should be prefixed with '\_' as 
`_xUpperCamelCase`. **Class attributes** must be preceded by 'c\_' as `c_xUpperCamelCase`.

___
## Functions and Methods
Functions and methods must be preceded by comments stating the **primary contributor**, the 
**parameters** and their **descriptions**, and the **returned values** and their **descriptions**.

Functions and methods will use `lowerCamelCase` naming. The parameter parenthesis should 
immediately follow, and parameters should have a space on either side. On definition the braces 
will open inline with the declaration and close in column with the declaration. As follows:

```c++
// Author: Spencer Barnes
// params: double array - list of grades to be averaged
//	   int - length of array
// return: double - average of array
double getAverage( double* adGrades, int iLength ) {
    double dAvg = 0;
    for( int i = 0; i < iLength; i++ ) {
        dAvg += adGrades[i];
    }
    return dAvg / iLength;
}
```
```c++
dTestAvg = getAverage( adTestGrades, iTestGradesLength );
```

___
## Classes
Must be named using `UpperCamelCase`. They follow the same brace rules as methods. For python,
a description of the class's attributes should commented at the top of the class.