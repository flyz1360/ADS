//
// Test.cpp
//
// This is a direct port of the C version of the RTree test program.
//
#include <opencv2\opencv.hpp>
#include "opencv2/core/core.hpp"
#include <string>
#include <iostream>
#include "ImageHandler.h"
#include"ColorHistogram.h"
#include"ShapeFourierDescriptor.h"
#include "FileDirectory.h"
#include<fstream>
using namespace std;
using namespace cv;

bool MySearchCallback(int id, void* arg) 
{
  cout << "Hit data rect "<< id+1<< endl;
  return true; // keep going
}

void constructImageIndexFromFeatureFile(RTree<int,double,8,double> &rtree ,char* inFeatureFilePath, int dim)
{
	 ifstream fileIn(inFeatureFilePath, ios::in);
	 //ofstream fileOut("G:\/ѧҵ��H\/�����¿γ�\/�߼����ݽṹ\/�γ���ҵ\/ADS-Project1-Release\/ADS-Project1-Release\/data\/color_feature_double.txt", ios::app);
	 int i = 0;
	 
	 string feature;
	 ImageFeature imageFeature;
	 ImageHandler imageHandler;
	 char buffer[80];
	
	 while(!fileIn.eof())
	 {
		 
		 fileIn.getline(buffer,80);
		 feature = buffer;

		 if(feature.length() > 0 && feature[0] == 'r')
		 {
			 imageFeature = imageHandler.parseImageFeature(feature,dim);
			 rtree.Insert(imageFeature.data, imageFeature.data,i);  //��������ӵ�RTree��
			 i++;
		 }
		 
	 }
	 fileIn.close();
	
}


void main()
{
	char wndname[] = "Drawing Demo";
	const int NUMBER = 100;
	const int DELAY = 5;
	int lineType = CV_AA; // change it to 8 to see non-antialiased graphics
	int i, width = 1000, height = 700;
	int x1 = 0, x2 = width, y1 = 0, y2 = height;
	RNG rng(0xFFFFFFFF);

	Mat image = Mat::zeros(height, width, CV_8UC3);
	imshow(wndname, image);
	for (i = 0; i < 100; i++)
	{
		Point center;
		center.x = rng.uniform(x1, x2);
		center.y = rng.uniform(y1, y2);

		circle(image, center,1, Scalar(255, 255, 255),
			1,  lineType);

		imshow(wndname, image);
	}
	waitKey();


	//1.ͼ���ļ���صĴ���
	//ImageHandler imageHandler = ImageHandler();  //��ʼ��ͼ������  
	
	int dimension = 2;//��ɫֱ��ͼ��ά��
	//imageHandler.inputImageInformation(imageListFile);//����ͼ����Ϣ



	//2.��ȡ������Ϣ:ͨ�ã�ֻҪ����һ��������ȡ��������һ��������
	//imageHandler.exportFeatrueFile(extractShapeFourierDescriptor,imageListFile,fileDepositoryPath,outFeatureFile,dimension);
	

	//3.��������
	//rtreeģ�潫ά�Ȱ�����ȥ�ˣ����ÿ�α任ζ����ʱ�򣬾ȵð�r�������޸�һ�飬�Һ��潫ά�ȴ�ģ���������������ʹ��ɶ�ָ̬��
	/*
	RTree<int,double,8,double> rtree;
	constructImageIndexFromFeatureFile(rtree,outFeatureFile,8);  //��ȡ�����ļ����ݣ���������


	//4.k������ȡ
	int k = 10;
	double feature[8] ={529.241, 26.9598, 1000, 550.809, 0.414766, 0, 7.88055, 217.752};
	int* nnList = rtree.KNN(feature,k);
	string* nameList = new string[k];
	int i;
	for(i = 0; i <k; i++)
	{
		nameList[i] = imageHandler.imageList[nnList[i]];
		cout << i<<": "<< nameList[i] << endl;
	}
	*/


	//������״������ȡ����
	/*
	IplImage* Img1 = cvLoadImage("G://ѧҵ��H//�����¿γ�//�߼����ݽṹ//�γ���ҵ//ADS-Project1-Release//ADS-Project1-Release//data//image//image//n01613177_610.JPEG");
	
	IplImage* Img2 = cvCreateImage(cvGetSize(Img1), Img1->depth,Img1->nChannels);
    cvCopy(Img1, Img2, NULL);

	cvReleaseImage(&Img1);
	ImageFeature feature = extractShapeFourierDescriptor(Img2,2);
	for(int i = 0; i <feature.dimNo; i++)
	{
		cout << feature.data[i] << "  ";
	}*/

	/*
	cvNamedWindow("mainWin", CV_WINDOW_AUTOSIZE); //���崰��
    cvShowImage("mainWin", Img2 );   //�ڴ�����ʾ
	cvReleaseImage(&Img2);*/
}