#pragma once
#include "stdafx.h"

class ProcessImage
{
public:
	ProcessImage(wchar_t*);
	ProcessImage(byte *data, int width, int height, size_t stride);
	int FindMark(wchar_t *pi_TemplateName);

	~ProcessImage();
};

