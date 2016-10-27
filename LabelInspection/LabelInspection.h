#pragma once

#include "resource.h"

#define MENU_BUTTON_CHANGECAM 1
#define MENU_BUTTON_REF_IMAGE 2
#define MENU_BUTTON_EXCLUDE_AREAS 3
#define MENU_BUTTON_MIN_ERR_SIZE 4
#define MENU_STAT_WINDOW 5
#define WIDTH_CAMERA_EXCLUDE 300
#define HEIGHT_CAMERA_EXCLUDE 64

#define MAX_LOADSTRING 100
#define MAX_CAMERAS 4
#define MAX_RECTANGLES 5
#define REF_RECTANGLE 0
#define ZONE_RECTANGLE 1


#define SETUPTHREAD_STACK_SIZE 0x8000

static bool AcquisitionInProgress = false;
static bool downPointSet = false;

