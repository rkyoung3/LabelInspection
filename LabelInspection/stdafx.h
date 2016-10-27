// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <Strsafe.h> 
#include <stdio.h>
#include <windows.h>  
#include <wingdi.h>
#include <vfw.h>  
#include <time.h>
#include <wchar.h>
#include <cwchar>
#include <dshow.h>
#include <vector>
#include <objidl.h>
#include <gdiplus.h>


// OpenCV
#include "opencv2\core\core.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\imgcodecs\imgcodecs.hpp"
#include "opencv2\highgui\highgui.hpp"
#include <iostream>
#include "ProcessImage.h"
#include "LabelInspection.h"


using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

// TODO: reference additional headers your program requires here
