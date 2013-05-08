//
// Test.cpp
//
// This is a direct port of the C version of the RTree test program.
//
#include <opencv2\opencv.hpp>
#include <string>
#include <iostream>
#include "ImageHandler.h"
#include"ColorHistogram.h"
#include"ShapeFourierDescriptor.h"
#include "FileDirectory.h"
#include<fstream>
#define DIM 9

using namespace std;
using namespace cv;

int searchTime;

bool MySearchCallback(int id, void* arg) 
{
  cout << "Hit data rect "<< id+1<< endl;
  return true; // keep going
}

void constructImageIndexFromFeatureFile(RTree<int,double,DIM,double> &rtree ,char* inFeatureFilePath, int dim)
{
	 ifstream fileIn(inFeatureFilePath, ios::in);
	 //ofstream fileOut("G:\/学业卷H\/大三下课程\/高级数据结构\/课程作业\/ADS-Project1-Release\/ADS-Project1-Release\/data\/color_feature_double.txt", ios::app);
	 int i = 0;

	 string feature;
	 ImageFeature imageFeature;
	 ImageHandler imageHandler;
	 char buffer[500];

	 cout<<"输入数据集的个数"<<endl;
	 cin>>searchTime;

	 int time = searchTime;
	 while(!fileIn.eof() && time>0)
	 {

		 fileIn.getline(buffer,500);
		 feature = buffer;

		 if(feature.length() > 0 && feature[0] == 'r')
		 {
			 imageFeature = imageHandler.parseImageFeature(feature,dim);
			 rtree.Insert(imageFeature.data,imageFeature.data,i);  //将数据添加到RTree中
			 //rtree.Insert(imageFeature.data,i);  //将数据添加到R+Tree中
			 i++;
			 time--;
		 }

	 }
	 printf("%d\n", i);
	 fileIn.close();

}

bool searchImageIndexFromFeatureFile(RTree<int,double,DIM,double> &rtree ,char* getFeatureFilePath, int dim)
{
	ifstream fileIn(getFeatureFilePath, ios::in);
	ofstream fileout(OutPutFile, ios::out);
	int i = 0;

	string feature;
	ImageFeature imageFeature;
	ImageHandler imageHandler;
	char buffer[500];

	while(!fileIn.eof() && searchTime>0)
	{

		fileIn.getline(buffer,500);
		feature = buffer;

		if(feature.length() > 0 && feature[0] == 'r')
		{
			imageFeature = imageHandler.parseImageFeature(feature,dim);
			//rtree.Insert(imageFeature.data, imageFeature.data,i);  //将数据添加到RTree中
			rtree.Search(imageFeature.data, imageFeature.data, MySearchCallback, NULL);
			i++;

			printf("%d\n", rtree.visitCount);
			fileout<<rtree.visitCount<<endl;

			rtree.visitCount = 0;


			searchTime--;
		}

	}
	printf("%d\n", i);
	fileIn.close();
	fileout.close();

	return 1;
}

void drawRTree(RTree<int,double,DIM,double> &rtree)
{
	char wndname[] = "Drawing Demo";
	const int DELAY = 5;
	int i, width =1000, height = 1000;
	int x1 = 0, x2 = width, y1 = 0, y2 = height;
	RNG rng(0xFFFFFFFF);
	
	Mat image = Mat::zeros(height, width, CV_8UC3);
	
	rtree.drawNode(rtree.m_root, &image, 3);

	imshow(wndname, image);
	waitKey();
}

void main()
{
		////1.图像文件相关的处理
	//ImageHandler imageHandler = ImageHandler();  //初始化图像处理类  
	//
	//int dimension = 2;//颜色直方图的维度
	//imageHandler.inputImageInformation(imageListFile);//输入图像信息

	////2.提取特征信息:通用，只要传入一个特征提取函数（第一个参数）
	//imageHandler.exportFeatrueFile(extractShapeFourierDescriptor,imageListFile,fileDepositoryPath,outFeatureFile,dimension);

	//3.建立索引
	//rtree模版将维度包含进去了，因此每次变换味道的时候，救得把r树声明修改一遍，我后面将维度从模版声明分离出来，使其可动态指定
	RTree<int,double,DIM,double> rtree;
	constructImageIndexFromFeatureFile(rtree,FeatureFile,DIM);  //读取特征文件数据，建立索引

	// 可视化R树索引
	//drawRTree(rtree);

	/*
	//4.k近邻求取
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


	//搜索性能测试
	searchImageIndexFromFeatureFile(rtree, FeatureFile, DIM);



	//进行形状特征提取测试
	/*
	IplImage* Img1 = cvLoadImage("G://学业卷H//大三下课程//高级数据结构//课程作业//ADS-Project1-Release//ADS-Project1-Release//data//image//image//n01613177_610.JPEG");
	
	IplImage* Img2 = cvCreateImage(cvGetSize(Img1), Img1->depth,Img1->nChannels);
    cvCopy(Img1, Img2, NULL);

	cvReleaseImage(&Img1);
	ImageFeature feature = extractShapeFourierDescriptor(Img2,2);
	for(int i = 0; i <feature.dimNo; i++)
	{
		cout << feature.data[i] << "  ";
	}*/

	/*
	cvNamedWindow("mainWin", CV_WINDOW_AUTOSIZE); //定义窗口
    cvShowImage("mainWin", Img2 );   //在窗口显示
	cvReleaseImage(&Img2);*/
	getchar();
	getchar();
}