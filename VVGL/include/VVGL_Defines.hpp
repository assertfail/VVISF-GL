#ifndef VVGL_Defines_h
#define VVGL_Defines_h



/*

developers need to define one of these during compilation:

	ISF_SDK_MAC
	ISF_SDK_IOS
	ISF_SDK_RPI
	ISF_SDK_GLFW
	ISF_SDK_QT

this header file defines a series of macros that describe the availability of various GL 
environments based on the SDK as defined by the user.  these are the possible values- note that 
these are not mutually exclusive- more than one may be defined simultaneously, these are only used 
to block of function calls that won't compile/link against a given SDK's headers/libs.

	ISF_TARGETENV_GL2
	ISF_TARGETENV_GL3PLUS
	ISF_TARGETENV_GLES
	ISF_TARGETENV_GLES3



*/




#if ISF_SDK_MAC
	#define ISF_TARGETENV_GL2
	#define ISF_TARGETENV_GL3PLUS
#elif ISF_SDK_IOS
	#define ISF_TARGETENV_GLES3
#elif ISF_SDK_RPI
	#define ISF_TARGETENV_GLES
#elif ISF_SDK_GLFW
	//	not sure, need to test this
	#define ISF_TARGETENV_GL3PLUS
#elif ISF_SDK_QT
	#define ISF_TARGETENV_GL2
	#define ISF_TARGETENV_GL3PLUS
#endif




#if ISF_SDK_QT
#include "VVGL_Qt_global.h"
#endif




#endif /* VVGL_Defines_h */