//
// Test.cpp
//
// This is a direct port of the C version of the RTree test program.
//
#include <opencv2\opencv.hpp>
#include <string>
#include <iostream>
#include "ImageHandler.h"
#include<fstream>
using namespace std;
using namespace cv;

bool MySearchCallback(int id, void* arg) 
{
  printf("Hit data rect %d\n", id+1);
  return true; // keep going
}

void constructImageIndexFromFeatureFile(RTree<int,double,8,double> &rtree ,char* inFeatureFilePath, int dim)
{
	 ifstream fileIn(inFeatureFilePath, ios::in);
	 //ofstream fileOut("G:\/学业卷H\/大三下课程\/高级数据结构\/课程作业\/ADS-Project1-Release\/ADS-Project1-Release\/data\/color_feature_double.txt", ios::app);
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
			 rtree.Insert(imageFeature.data, imageFeature.data,i);  //将数据添加到RTree中
			 i++;
		 }
		 
	 }
	 fileIn.close();
	
}


void main()
{
	/**1.图像文件相关的处理*/
	ImageHandler imageHandler = ImageHandler();  //初始化图像处理类
	      //下面是图像文件存储路径
	char* imageListFile = "G:\/学业卷H\/大三下课程\/高级数据结构\/课程作业\/ADS-Project1-Release\/ADS-Project1-Release\/data\/imagelist.txt";
	char* fileDepositoryPath = "G:\/学业卷H\/大三下课程\/高级数据结构\/课程作业\/ADS-Project1-Release\/ADS-Project1-Release\/data\/image\/image";
	     //将提取后的特征放在这个文件里
	char* outFeatureFile= "G:\/学业卷H\/大三下课程\/高级数据结构\/课程作业\/ADS-Project1-Release\/ADS-Project1-Release\/data\/color_dim2_norm1000.txt";
	


	int dimension = 2;//颜色直方图的维度
	
	imageHandler.inputImageInformation(imageListFile);//输入图像信息

	/**2.提取特征信息*/
	/*imageHandler.exportRGBColorHistogramFile(imageListFile,fileDepositoryPath,outFeatureFile,dimension);*/

	

	/**3.建立索引*/
	/*rtree模版将维度包含进去了，因此每次变换味道的时候，救得把r树声明修改一遍，我后面将维度从模版声明分离出来，使其可动态指定*/
	
	RTree<int,double,8,double> rtree;
	constructImageIndexFromFeatureFile(rtree,outFeatureFile,8);  //读取特征文件数据，建立索引


	/**4.k近邻求取*/
	
	int k = 10;
	double feature[8] ={687.919, 588.589, 7.78489, 553.245, 9.38452, 6.15478, 0, 1000};
	int* nnList = rtree.KNN(feature,k);
	string* nameList = new string[k];
	int i;
	for(i = 0; i <k; i++)
	{
		nameList[i] = imageHandler.imageList[nnList[i]];
		cout << i<<": "<< nameList[i] << endl;
	}
	

    cout << "Press any key to continue..."<< endl;
    getchar(); // Wait for keypress on exit so we can read console output
}