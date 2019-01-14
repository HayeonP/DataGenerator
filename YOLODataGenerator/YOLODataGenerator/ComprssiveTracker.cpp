#include "CompressiveTracker.h"
#include <math.h>
#include <iostream>
#include <ctime>


using namespace cv;
using namespace std;

//------------------------------------------------
CompressiveTracker::CompressiveTracker(void)
{
	featureMinNumRect = 2;
	featureMaxNumRect = 5;	// number of rectangle from 2 to 4
	featureNum = 1000;	// number of all weaker classifiers, i.e,feature pool
	rOuterPositive = 4;	// radical scope of positive samples
	rSearchWindow = 25; // size of search window
	muPositive = vector<float>(featureNum, 0.0f);
	muNegative = vector<float>(featureNum, 0.0f);
	sigmaPositive = vector<float>(featureNum, 1.0f);
	sigmaNegative = vector<float>(featureNum, 1.0f);
	learnRate = 0.85f;	// Learning rate parameter


						//Added by Sso
	std::time_t time = std::time(nullptr);
	rng.state = (uint64)time;
	fScaleRatio = 1.0f;
	fScaleStep = 0.2f;
	nFrame = 0;
}

CompressiveTracker::~CompressiveTracker(void)
{
}


void CompressiveTracker::HaarFeature(Rect& _objectBox, int _numFeature)
/*Description: compute Haar features
Arguments:
-_objectBox: [x y width height] object rectangle
-_numFeature: total number of features.The default is 50.
*/
{
	features = vector<vector<Rect>>(_numFeature, vector<Rect>());
	featuresWeight = vector<vector<float>>(_numFeature, vector<float>());

	int numRect;
	Rect rectTemp;
	float weightTemp;

	for (int i = 0; i<_numFeature; i++)
	{
		numRect = cvFloor(rng.uniform((double)featureMinNumRect, (double)featureMaxNumRect));

		//int c = 1;
		for (int j = 0; j<numRect; j++)
		{
			rectTemp.x = cvFloor(rng.uniform(2.0, (double)(_objectBox.width - 5)));
			rectTemp.y = cvFloor(rng.uniform(2.0, (double)(_objectBox.height - 5)));
			rectTemp.width = cvCeil(rng.uniform(2.0, (double)(_objectBox.width - rectTemp.x - 2)));
			rectTemp.height = cvCeil(rng.uniform(2.0, (double)(_objectBox.height - rectTemp.y - 2)));
			features[i].push_back(rectTemp);

			weightTemp = (float)pow(-1.0, cvFloor(rng.uniform(0.0, 2.0))) / sqrt(float(numRect));
			//			weightTemp = (float)pow(-1.0, cvFloor(rng.uniform(0.0, 2.0))) / (rectTemp.width*rectTemp.height);

			featuresWeight[i].push_back(weightTemp); //set a bias for haar-like features and a random matrix.
		}
	}
}

void CompressiveTracker::sampleRect(Mat& _image, Rect& _objectBox, float _rInner, float _rOuter, int _maxSampleNum, vector<Rect>& _sampleBox)
/* Description: compute the coordinate of positive and negative sample image templates
Arguments:
-_image:        processing frame
-_objectBox:    recent object position
-_rInner:       inner sampling radius
-_rOuter:       Outer sampling radius
-_maxSampleNum: maximal number of sampled images
-_sampleBox:    Storing the rectangle coordinates of the sampled images.
*/
{
	int rowsz = _image.rows - _objectBox.height - 1;
	int colsz = _image.cols - _objectBox.width - 1;
	float inradsq = _rInner*_rInner;
	float outradsq = _rOuter*_rOuter;

	float dist;
	/*
	int minrow = max(0,(int)_objectBox.y-(int)_rInner);
	int maxrow = min((int)rowsz-1,(int)_objectBox.y+(int)_rInner);
	int mincol = max(0,(int)_objectBox.x-(int)_rInner);
	int maxcol = min((int)colsz-1,(int)_objectBox.x+(int)_rInner);
	*/
	float minrow = MAXF(1, (float)_objectBox.y - (float)_rInner + 1);
	float maxrow = MINF((float)rowsz - 1, (float)_objectBox.y + (float)_rInner);
	float mincol = MAXF(1, (float)_objectBox.x - (float)_rInner + 1);
	float maxcol = MINF((float)colsz - 1, (float)_objectBox.x + (float)_rInner);

	//float minrow = MAXF(1, (float)_objectBox.y + (float)_rInner + 1);
	//float maxrow = MINF((float)rowsz - 1, (float)_objectBox.y + (float)_objectBox.height - (float)_rInner);
	//float mincol = MAXF(1, (float)_objectBox.x + (float)_rInner + 1);
	//float maxcol = MINF((float)colsz - 1, (float)_objectBox.x + (float)_objectBox.width - (float)_rInner);

	int i = 0;

	float prob = ((float)(_maxSampleNum)) / (maxrow - minrow + 1) / (maxcol - mincol + 1);

	float r;
	float c;

	_sampleBox.clear();//important
	Rect rec(0, 0, 0, 0);

	for (r = minrow; r <= (float)maxrow; r += 1)
		for (c = mincol; c <= (float)maxcol; c += 1) {
			dist = (_objectBox.y - r)*(_objectBox.y - r) + (_objectBox.x - c)*(_objectBox.x - c);

			if (rng.uniform(0., 1.)<prob && dist < inradsq && dist >= outradsq) {

				rec.x = c;
				rec.y = r;
				rec.width = _objectBox.width;
				rec.height = _objectBox.height;

				_sampleBox.push_back(rec);

				i++;
			}
		}

	_sampleBox.resize(i);
}

void CompressiveTracker::sampleRect(Mat& _image, Rect& _objectBox, float _srw, vector<Rect>& _sampleBox)
/* Description: Compute the coordinate of samples when detecting the object.*/
{
	int rowsz = _image.rows - _objectBox.height - 1;
	int colsz = _image.cols - _objectBox.width - 1;
	float inradsq = _srw*_srw;

	float dist;
	/*
	int minrow = max(0,(int)_objectBox.y-(int)_rInner);
	int maxrow = min((int)rowsz-1,(int)_objectBox.y+(int)_rInner);
	int mincol = max(0,(int)_objectBox.x-(int)_rInner);
	int maxcol = min((int)colsz-1,(int)_objectBox.x+(int)_rInner);
	*/

	float minrow = MAXF(1, (float)_objectBox.y - (float)_srw + 1);
	float maxrow = MINF((float)rowsz - 1, (float)_objectBox.y + (float)_srw);
	float mincol = MAXF(1, (float)_objectBox.x - (float)_srw + 1);
	float maxcol = MINF((float)colsz - 1, (float)_objectBox.x + (float)_srw);

	//float minrow = MAXF(1, (float)_objectBox.y + (float)_srw + 1);
	//float maxrow = MINF((float)rowsz - 1, (float)_objectBox.y + (float)_objectBox.height - (float)_srw);
	//float mincol = MAXF(1, (float)_objectBox.x + (float)_srw + 1);
	//float maxcol = MINF((float)colsz - 1, (float)_objectBox.x + (float)_objectBox.width - (float)_srw);

	int i = 0;
	float r;
	float c;

	_sampleBox.clear();//important
	Rect rec(0, 0, 0, 0);

	for (r = minrow; r <= (float)maxrow; r += 1)
		for (c = mincol; c <= (float)maxcol; c += 1) {
			dist = (_objectBox.y - r)*(_objectBox.y - r) + (_objectBox.x - c)*(_objectBox.x - c);

			if (dist < inradsq) {

				rec.x = c;
				rec.y = r;
				rec.width = _objectBox.width;
				rec.height = _objectBox.height;

				_sampleBox.push_back(rec);

				i++;
			}
		}

	_sampleBox.resize(i);
}

// Compute the features of samples
void CompressiveTracker::getFeatureValue(Mat& _imageIntegral, vector<Rect>& _sampleBox, Mat& _sampleFeatureValue)
{
	int sampleBoxSize = _sampleBox.size();
	_sampleFeatureValue.create(featureNum, sampleBoxSize, CV_32F);
	float tempValue;
	int xMin;
	int xMax;
	int yMin;
	int yMax;

	for (int i = 0; i<featureNum; i++)
	{
		for (int j = 0; j<sampleBoxSize; j++)
		{
			tempValue = 0.0f;
			for (size_t k = 0; k<features[i].size(); k++)
			{
				xMin = _sampleBox[j].x - 1 + features[i][k].x;
				xMax = _sampleBox[j].x - 1 + features[i][k].x + features[i][k].width - 1;
				yMin = _sampleBox[j].y - 1 + features[i][k].y;
				yMax = _sampleBox[j].y - 1 + features[i][k].y + features[i][k].height - 1;
				tempValue += featuresWeight[i][k] *
					(_imageIntegral.at<float>(yMin, xMin) +
						_imageIntegral.at<float>(yMax, xMax) -
						_imageIntegral.at<float>(yMin, xMax) -
						_imageIntegral.at<float>(yMax, xMin)) /
						(features[i][k].width*features[i][k].height);
			}
			_sampleFeatureValue.at<float>(i, j) = tempValue;
		}
	}
}

// Update the mean and variance of the gaussian classifier
void CompressiveTracker::classifierUpdate(Mat& _sampleFeatureValue, vector<float>& _mu, vector<float>& _sigma, float _learnRate)
{
	Scalar muTemp;
	Scalar sigmaTemp;

	for (int i = 0; i<featureNum; i++)
	{
		meanStdDev(_sampleFeatureValue.row(i), muTemp, sigmaTemp);

		_sigma[i] = (float)sqrt(_learnRate*_sigma[i] * _sigma[i] + (1.0f - _learnRate)*sigmaTemp.val[0] * sigmaTemp.val[0]
			+ _learnRate*(1.0f - _learnRate)*(_mu[i] - muTemp.val[0])*(_mu[i] - muTemp.val[0]));	// equation 6 in paper

		_mu[i] = _mu[i] * _learnRate + (1.0f - _learnRate)*muTemp.val[0];	// equation 6 in paper
	}
}

// Compute the ratio classifier 
void CompressiveTracker::ratioClassifier(vector<float>& _muPos, vector<float>& _sigmaPos, vector<float>& _muNeg, vector<float>& _sigmaNeg,
	Mat& _sampleFeatureValue, float& _ratioMax, int& _ratioMaxIndex)
{
	float sumratio;
	_ratioMax = -FLT_MAX;
	_ratioMaxIndex = 0;
	float pPos;
	float pNeg;
	int sampleBoxNum = _sampleFeatureValue.cols;

	for (int j = 0; j<sampleBoxNum; j++)
	{
		sumratio = 0.0f;
		for (int i = 0; i<featureNum; i++)
		{
			pPos = exp((_sampleFeatureValue.at<float>(i, j) - _muPos[i])*(_sampleFeatureValue.at<float>(i, j) - _muPos[i]) / -(2.0f*_sigmaPos[i] * _sigmaPos[i] + 1e-30)) / (_sigmaPos[i] + 1e-30);
			pNeg = exp((_sampleFeatureValue.at<float>(i, j) - _muNeg[i])*(_sampleFeatureValue.at<float>(i, j) - _muNeg[i]) / -(2.0f*_sigmaNeg[i] * _sigmaNeg[i] + 1e-30)) / (_sigmaNeg[i] + 1e-30);
			sumratio += log(pPos + 1e-30) - log(pNeg + 1e-30);	// equation 4
		}

		if (_ratioMax < sumratio)
		{
			_ratioMax = sumratio;
			_ratioMaxIndex = j;
		}
	}
}

// Compute the ratio classifier 
void CompressiveTracker::ratioClassifier(vector<float>& _muPos, vector<float>& _sigmaPos, vector<float>& _muNeg, vector<float>& _sigmaNeg,
	Mat& _sampleFeatureValue, float& _ratioMax, int& _ratioMaxIndex, vector<float> &vRatios)
{
	float sumratio;
	_ratioMax = -FLT_MAX;
	_ratioMaxIndex = 0;
	float pPos;
	float pNeg;
	int sampleBoxNum = _sampleFeatureValue.cols;

	for (int j = 0; j<sampleBoxNum; j++)
	{
		sumratio = 0.0f;
		for (int i = 0; i<featureNum; i++)
		{
			pPos = exp((_sampleFeatureValue.at<float>(i, j) - _muPos[i])*(_sampleFeatureValue.at<float>(i, j) - _muPos[i]) / -(2.0f*_sigmaPos[i] * _sigmaPos[i] + 1e-30)) / (_sigmaPos[i] + 1e-30);
			pNeg = exp((_sampleFeatureValue.at<float>(i, j) - _muNeg[i])*(_sampleFeatureValue.at<float>(i, j) - _muNeg[i]) / -(2.0f*_sigmaNeg[i] * _sigmaNeg[i] + 1e-30)) / (_sigmaNeg[i] + 1e-30);
			sumratio += log(pPos + 1e-30) - log(pNeg + 1e-30);	// equation 4
		}
		vRatios.push_back(sumratio);

		if (_ratioMax < sumratio)
		{
			_ratioMax = sumratio;
			_ratioMaxIndex = j;
		}
	}
}

void CompressiveTracker::init(Mat& _frame, Rect& _objectBox)
{
	// compute feature template
	HaarFeature(_objectBox, featureNum);


	// compute sample templates
	sampleRect(_frame, _objectBox, rOuterPositive, samplePositiveBox);
	//sampleRect(_frame, _objectBox, rOuterPositive, 0, 1000000, samplePositiveBox);
	sampleRect(_frame, _objectBox, rSearchWindow, rOuterPositive, 100, sampleNegativeBox);

	//int i;
	//Mat matTempImage; cvtColor(_frame, matTempImage, CV_GRAY2BGR);
	//for (i = 0; i<sampleNegativeBox.size(); i++)
	//	rectangle(matTempImage, sampleNegativeBox[i], Scalar(255, 0, 0));
	//for (i = 0; i<samplePositiveBox.size(); i++)
	//	rectangle(matTempImage, samplePositiveBox[i], Scalar(0, 0, 255));
	//imshow("Init", matTempImage);
	//waitKey(30);
	integral(_frame, imageIntegral, CV_32F);

	getFeatureValue(imageIntegral, samplePositiveBox, samplePositiveFeatureValue);
	getFeatureValue(imageIntegral, sampleNegativeBox, sampleNegativeFeatureValue);
	classifierUpdate(samplePositiveFeatureValue, muPositive, sigmaPositive, learnRate);
	classifierUpdate(sampleNegativeFeatureValue, muNegative, sigmaNegative, learnRate);

	rectInit = _objectBox;
}
void CompressiveTracker::processFrame(Mat& _frame, Rect& _objectBox)
{
	// predict
	sampleRect(_frame, _objectBox, rSearchWindow, detectBox);
	integral(_frame, imageIntegral, CV_32F);
	getFeatureValue(imageIntegral, detectBox, detectFeatureValue);
	int ratioMaxIndex;
	float ratioMax;
	ratioClassifier(muPositive, sigmaPositive, muNegative, sigmaNegative, detectFeatureValue, ratioMax, ratioMaxIndex);
	_objectBox = detectBox[ratioMaxIndex];

	// update
	//	sampleRect(_frame, _objectBox, rOuterPositive, 0.0, 1000000, samplePositiveBox);
	sampleRect(_frame, _objectBox, rOuterPositive, samplePositiveBox);
	sampleRect(_frame, _objectBox, rSearchWindow*1.5, rOuterPositive + 4.0, 50, sampleNegativeBox);

	getFeatureValue(imageIntegral, samplePositiveBox, samplePositiveFeatureValue);
	getFeatureValue(imageIntegral, sampleNegativeBox, sampleNegativeFeatureValue);
	classifierUpdate(samplePositiveFeatureValue, muPositive, sigmaPositive, learnRate);
	classifierUpdate(sampleNegativeFeatureValue, muNegative, sigmaNegative, learnRate);
}

void CompressiveTracker::fastSampleRect(Mat& _image, Rect& _objectBox, float _srw, int nStep, vector<Rect>& _sampleBox)
{
	int rowsz = _image.rows - _objectBox.height - 1;
	int colsz = _image.cols - _objectBox.width - 1;
	float inradsq = _srw*_srw;


	float dist;
	/*
	int minrow = max(0,(int)_objectBox.y-(int)_rInner);
	int maxrow = min((int)rowsz-1,(int)_objectBox.y+(int)_rInner);
	int mincol = max(0,(int)_objectBox.x-(int)_rInner);
	int maxcol = min((int)colsz-1,(int)_objectBox.x+(int)_rInner);
	*/

	float minrow = MAXF(1, (float)_objectBox.y - (float)_srw + 1);
	float maxrow = MINF((float)rowsz - 1, (float)_objectBox.y + (float)_srw);
	float mincol = MAXF(1, (float)_objectBox.x - (float)_srw + 1);
	float maxcol = MINF((float)colsz - 1, (float)_objectBox.x + (float)_srw);

	int i = 0;
	float r;
	float c;

	_sampleBox.clear();//important
	Rect rec(0, 0, 0, 0);

	for (r = minrow; r <= (float)maxrow; r += nStep) {
		for (c = mincol; c <= (float)maxcol; c += nStep) {
			dist = (_objectBox.y - r)*(_objectBox.y - r) + (_objectBox.x - c)*(_objectBox.x - c);

			if (dist < inradsq) {

				rec.x = c;
				rec.y = r;
				rec.width = _objectBox.width;
				rec.height = _objectBox.height;

				_sampleBox.push_back(rec);

				i++;
			}
		}
	}

	_sampleBox.resize(i);
}

void CompressiveTracker::fastProcessFrame(Mat& _frame, Rect& _objectBox)
{
	int ratioMaxIndex;
	float ratioMax;

	integral(_frame, imageIntegral, CV_32F);

	// Coarse Detection
	fastSampleRect(_frame, _objectBox, 15, 3, detectBox);
	getFeatureValue(imageIntegral, detectBox, detectFeatureValue);

	//vector<float> vRatios;
	//ratioClassifier(muPositive, sigmaPositive, muNegative, sigmaNegative, detectFeatureValue, ratioMax, ratioMaxIndex, vRatios);
	ratioClassifier(muPositive, sigmaPositive, muNegative, sigmaNegative, detectFeatureValue, ratioMax, ratioMaxIndex);
	_objectBox = detectBox[ratioMaxIndex];

	//int i;
	//Mat matTempImage;
	//string strText;
	//for (i = 0; i < detectBox.size(); i++)
	//{
	//	strText = format("%d, %f", i, vRatios[i]);
	//	cvtColor(_frame, matTempImage, CV_GRAY2BGR);
	//	rectangle(matTempImage, detectBox[i], Scalar(0, 0, 255), 2);
	//	putText(matTempImage, strText, Point(10 ,50), CV_FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	//	imshow("c", matTempImage);
	//	waitKey(0);
	//}

	// Fine Detection
	fastSampleRect(_frame, _objectBox, 10, 1, detectBox);
	getFeatureValue(imageIntegral, detectBox, detectFeatureValue);

	ratioClassifier(muPositive, sigmaPositive, muNegative, sigmaNegative, detectFeatureValue, ratioMax, ratioMaxIndex);
	_objectBox = detectBox[ratioMaxIndex];

	//cvtColor(_frame, matTempImage, CV_GRAY2BGR);
	//for (i = 0; i < detectBox.size(); i++)
	//{
	//	rectangle(matTempImage, detectBox[i], Scalar(0, 0, 255), 2);
	//}
	//imshow("f", matTempImage);

	// update
	//	sampleRect(_frame, _objectBox, rOuterPositive, 0.0, 1000000, samplePositiveBox);
	//sampleRect(_frame, _objectBox, rOuterPositive, samplePositiveBox);
	//sampleRect(_frame, _objectBox, rSearchWindow*1.5, rOuterPositive+4.0, 100, sampleNegativeBox);
	//getFeatureValue(imageIntegral, samplePositiveBox, samplePositiveFeatureValue);
	//getFeatureValue(imageIntegral, sampleNegativeBox, sampleNegativeFeatureValue);
	//classifierUpdate(samplePositiveFeatureValue, muPositive, sigmaPositive, learnRate);
	//classifierUpdate(sampleNegativeFeatureValue, muNegative, sigmaNegative, learnRate);
}

void CompressiveTracker::scaledFastProcessFrame(Mat& _frame, Rect& _objectBox)
{
	int i, j;
	int ratioMaxIndex;
	vector<vector<Rect>> vPatches(3);
	vector<float> vScaleRatio(3); // 0: small, 1: current, 2: large
	vector<Rect> vScaledRect(3);
	vector<Mat> vScaledDetectFeatureValues(3);
	Point ptBoxCenter;
	Size sizeScaledBox;
	Rect rectScaledBox, rectOptimalBox;

	float scale, ratioMax, ratioSmallScale, ratioLargeScale, scaleStep;
	Rect rectSmallBox, rectLargeBox;

	integral(_frame, imageIntegral, CV_32F);

	// Coarse Detection
	fastSampleRect(_frame, _objectBox, 25, 4, detectBox);
	getFeatureValue(imageIntegral, detectBox, detectFeatureValue);

	ratioClassifier(muPositive, sigmaPositive, muNegative, sigmaNegative, detectFeatureValue, vScaleRatio[1], ratioMaxIndex);
	_objectBox = detectBox[ratioMaxIndex];

	//Fine Detection with Multi-Scale

	if (nFrame++ < 5)
	{
		fastSampleRect(_frame, _objectBox, 10, 1, detectBox);
		getFeatureValue(imageIntegral, detectBox, detectFeatureValue);

		ratioClassifier(muPositive, sigmaPositive, muNegative, sigmaNegative, detectFeatureValue, ratioMax, ratioMaxIndex);
		rectOptimalBox = detectBox[ratioMaxIndex];
	}
	else
	{
		nFrame = 0;
		ratioMax = -FLT_MAX;
		for (i = 0; i<3; i++)
		{
			//Box Scaling
			scale = fScaleRatio + (i - 1)*fScaleStep;
			ptBoxCenter = Point(_objectBox.x + _objectBox.width / 2, _objectBox.y + _objectBox.height / 2);
			sizeScaledBox = Size(_objectBox.width*scale, _objectBox.height*scale);
			rectScaledBox = Rect(max(0, ptBoxCenter.x - sizeScaledBox.width / 2), max(0, ptBoxCenter.y - sizeScaledBox.height / 2), sizeScaledBox.width, sizeScaledBox.height);



			fastSampleRect(_frame, rectScaledBox, 10, 1, vPatches[i]);
			getScaledFeatureValue(imageIntegral, vPatches[i], vScaledDetectFeatureValues[i], scale);

			ratioClassifier(muPositive, sigmaPositive, muNegative, sigmaNegative, vScaledDetectFeatureValues[i], vScaleRatio[i], ratioMaxIndex);
			vScaledRect[i] = vPatches[i][ratioMaxIndex];

			if (vScaleRatio[i] > ratioMax)
			{
				ratioMax = vScaleRatio[i];
				rectOptimalBox = vScaledRect[i];
			}
		}
	}

	_objectBox = rectOptimalBox;

	sampleRect(_frame, _objectBox, rOuterPositive, samplePositiveBox);
	sampleRect(_frame, _objectBox, rSearchWindow*1.5, rOuterPositive + 4.0, 50, sampleNegativeBox);

	getFeatureValue(imageIntegral, samplePositiveBox, samplePositiveFeatureValue);
	getFeatureValue(imageIntegral, sampleNegativeBox, sampleNegativeFeatureValue);
	classifierUpdate(samplePositiveFeatureValue, muPositive, sigmaPositive, learnRate);
	classifierUpdate(sampleNegativeFeatureValue, muNegative, sigmaNegative, learnRate);
}

void CompressiveTracker::getScaledFeatureValue(Mat& _imageIntegral, vector<Rect>& _sampleBox, Mat& _sampleFeatureValue, float _scaledRatio)
{
	vector<vector<Rect>> vScaledFeatures;

	int i, j, k;
	int sampleBoxSize = _sampleBox.size();
	_sampleFeatureValue.create(featureNum, sampleBoxSize, CV_32F);
	float tempValue;
	int xMin;
	int xMax;
	int yMin;
	int yMax;

	//Size Scaling
	std::copy(features.begin(), features.end(), std::back_inserter(vScaledFeatures));
	for (i = 0; i<featureNum; i++)
	{
		for (k = 0; k<vScaledFeatures[i].size(); k++)
		{
			vScaledFeatures[i][k] = Rect(vScaledFeatures[i][k].x*_scaledRatio, vScaledFeatures[i][k].y*_scaledRatio, vScaledFeatures[i][k].width*_scaledRatio, vScaledFeatures[i][k].height*_scaledRatio);
		}
	}

	for (i = 0; i<featureNum; i++)
	{
		for (j = 0; j<sampleBoxSize; j++)
		{
			tempValue = 0.0f;
			for (k = 0; k<vScaledFeatures[i].size(); k++)
			{
				xMin = _sampleBox[j].x - 1 + vScaledFeatures[i][k].x;
				xMax = _sampleBox[j].x - 1 + vScaledFeatures[i][k].x + vScaledFeatures[i][k].width - 1;
				yMin = _sampleBox[j].y - 1 + vScaledFeatures[i][k].y;
				yMax = _sampleBox[j].y - 1 + vScaledFeatures[i][k].y + vScaledFeatures[i][k].height - 1;
				tempValue += featuresWeight[i][k] *
					(_imageIntegral.at<float>(yMin, xMin) +
						_imageIntegral.at<float>(yMax, xMax) -
						_imageIntegral.at<float>(yMin, xMax) -
						_imageIntegral.at<float>(yMax, xMin)) /
						(vScaledFeatures[i][k].width*vScaledFeatures[i][k].height);
			}
			_sampleFeatureValue.at<float>(i, j) = tempValue;
		}
	}
}