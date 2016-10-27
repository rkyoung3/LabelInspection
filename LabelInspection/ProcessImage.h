#pragma once
#include "stdafx.h"

class ProcessImage
{
public:
	ProcessImage(wchar_t*);
	int FindMark(wchar_t *pi_TemplateName);

	~ProcessImage();
};

