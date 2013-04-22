//
// Test.cpp
//
// This is a direct port of the C version of the RTree test program.
//
#include <opencv2\opencv.hpp>
#include <string>
#include <iostream>
#include "ImageHandler.h"
using namespace std;
using namespace cv;

bool MySearchCallback(int id, void* arg) 
{
  printf("Hit data rect %d\n", id+1);
  return true; // keep going
}


void main()
{
	
	/**1.图像文件相关的处理*/
	ImageHandler imageHandler = ImageHandler();  //初始化图像处理类
	      //下面是图像文件存储路径
	char* imageListFile = "G:\/学业卷H\/大三下课程\/高级数据结构\/课程作业\/ADS-Project1-Release\/ADS-Project1-Release\/data\/imagelist.txt";
	char* fileDepositoryPath = "G:\/学业卷H\/大三下课程\/高级数据结构\/课程作业\/ADS-Project1-Release\/ADS-Project1-Release\/data\/image\/image";
	     //将提取后的特征放在这个文件里
	char* outFeatureFile= "G:\/学业卷H\/大三下课程\/高级数据结构\/课程作业\/ADS-Project1-Release\/ADS-Project1-Release\/data\/color_his1000.txt";
	
	/**2.提取特征信息*/
	/*imageHandler.exportColorHistogramFile(imageListFile,fileDepositoryPath,outFeatureFile,dimension);*/

	int dimension = 2;//颜色直方图的维度 2*2*2 = 8
	
	imageHandler.setImageListFilePath(imageListFile);
	imageHandler.inputImageInformation();//输入图像信息
	

	/**3.建立索引*/
	RTree<int,double,8,double> rtree;
	imageHandler.constructImageIndexFromFeatureFile(rtree,outFeatureFile,8);  //读取特征文件数据，建立索引


	/**4.k近邻求取*/
	int k = 10;
	double feature[8] ={675.969, 598.488, 0, 566.323, 0, 0, 0, 1000  };
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