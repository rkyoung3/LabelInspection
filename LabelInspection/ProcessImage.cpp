#include "stdafx.h"
/*
#include "ProcessImage.h"
#include "stdafx.h"
#include <Strsafe.h> 
#include <stdio.h>
#include <windows.h>  
#include <vfw.h>  
#include <time.h>
#include <wchar.h>
#include <cwchar>
#include <dshow.h>
#include <vector>

// OpenCV
#include "opencv2\core\core.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\imgcodecs\imgcodecs.hpp"
#include "opencv2\highgui\highgui.hpp"
#include <iostream>
*/

using namespace cv;
using namespace std;

/// Global Variables
Mat img; 
Mat templ; 
Mat result;
const char* image_window = "Source Image";
const char* result_window = "Result window";

int match_method;
int max_Trackbar = 5;

/// Function Headers

void MatchingMethod(int, void*);

ProcessImage::ProcessImage(wchar_t *pi_FileName)
{
	/*
	* Function: cvCreateMat(int rows, int cols, int type)
	* Parameters:
	* rows – Number of rows in the matrix
	* cols – Number of columns in the matrix
	* type – The type of the matrix elements in the form CV_<bit depth><S|U|F>C<number of channels>, where S=signed, U=unsigned, F=float.
	* For example: [CV _ 8UC1 means the elements are 8-bit unsigned and the there is 1 channel],
	* and:         [CV _ 32SC2 means the elements are 32-bit signed and there are 2 channels.
	*************************************************************************************************************************************/
	wstring ws(pi_FileName);
	// your new String
	string str(ws.begin(), ws.end());
	
	img = imread(str);
	// namedWindow("image", WINDOW_NORMAL);
	// imshow("image", image);
	// waitKey(0);
	

}


int ProcessImage::FindMark(wchar_t * pi_TemplateName)
{

	wstring ws2(pi_TemplateName);
	string strTmpl(ws2.begin(), ws2.end());


	/// Load image and template
	// img = imread(strImg, CV_LOAD_IMAGE_COLOR);
	templ = imread(strTmpl, CV_LOAD_IMAGE_COLOR);

	/// Create windows
	namedWindow(image_window, WINDOW_AUTOSIZE);
	namedWindow(result_window, WINDOW_AUTOSIZE);

	/// Create Trackbar
	const char* trackbar_label = "Method: \n 0: SQDIFF \n 1: SQDIFF NORMED \n 2: TM CCORR \n 3: TM CCORR NORMED \n 4: TM COEFF \n 5: TM COEFF NORMED";
	createTrackbar(trackbar_label, image_window, &match_method, max_Trackbar, MatchingMethod);

	MatchingMethod(0, 0);

	waitKey(0);
	return 0;
}

ProcessImage::~ProcessImage()
{
	destroyAllWindows();

}


/**
* @function MatchingMethod
* @brief Trackbar callback
*/
void MatchingMethod(int, void*)
{
	/// Source image to display
	Mat img_display;
	img.copyTo(img_display);

	/// Create the result matrix
	int result_cols = img.cols - templ.cols + 1;
	int result_rows = img.rows - templ.rows + 1;

	result.create(result_cols, result_rows, CV_32FC1);

	/// Do the Matching and Normalize
	matchTemplate(img, templ, result, match_method);
	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

	/// Localizing the best match with minMaxLoc
	double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
	cv::Point matchLoc;

	for (int k = 1; k <= 5; k++)
	{
		minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
		result.at<float>(minLoc.x, minLoc.y) = 1.0;
		result.at<float>(maxLoc.x, maxLoc.y) = 0.0;

		/// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
		if (match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED)
		{
			matchLoc = minLoc;
		}
		else
		{
			matchLoc = maxLoc;
		}

		/// Show me what you got
		rectangle(img_display, matchLoc, cv::Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);
		rectangle(result, matchLoc, cv::Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);
	}
	imshow(image_window, img_display);
	imshow(result_window, result);

	return;
}
