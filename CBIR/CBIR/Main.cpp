//
// Test.cpp
//
// This is a direct port of the C version of the RTree test program.
//

#include <iostream>
#include<opencv2\opencv.hpp>
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
	/*
	ImageHandler imageHandler = ImageHandler();
	RTree<int,double,9,double> rtree;  //string表示图像的数据表示为一个string
	imageHandler.setImageListFilePath("G:\/学业卷H\/大三下课程\/高级数据结构\/课程作业\/ADS-Project1-Release\/ADS-Project1-Release\/data\/imagelist.txt");
	imageHandler.inputImageInformation();
	imageHandler.constructImageIndexFromFeatureFile(rtree,"G:\/学业卷H\/大三下课程\/高级数据结构\/课程作业\/ADS-Project1-Release\/ADS-Project1-Release\/data\/color_feature.txt");
	//ColorComment cc= {0.3311, 0.5294, 0.5654, 0.3241, 0.2575, 0.3381, 0.3440, 0.2992, 0.3689};
	
	ColorComment cc= {2504, 6421, 6172,1007, 2296, 2308, 1530, 2581, 2792};

	//search test
	rtree.Search(cc.data,cc.data,MySearchCallback,NULL);
	
	//knn test
	int k = 100;
	int* nnList = rtree.KNN(cc.data,k);

	string* nameList = new string[k];
	int i;
	for(i = 0; i <k; i++)
	{
		nameList[i] = imageHandler.imageList[nnList[i]];
		cout << i<<": "<< nameList[i] << endl;
	}
	*/

    /*
	ImageTag imageTag = imageHandler.parseImageName("n03877845_4630.JPEG");
	cout << imageTag.catagory << ", " << imageTag.index << endl;

	ColorComment cc = imageHandler.parserImageCommentFeature("rect 2138 2668 5303 2279 3243 3656 2681 3789 3922");
	for(int i = 0; i < 9; i++)
	{
		cout << cc.data[i] <<",";
	}
	cout << endl;*/
	/*
	ColorHistogram colHis = imageHandler.extractColorHostogram("G:\学业卷H\大三下课程\高级数据结构\课程作业\ADS-Project1-Release\ADS-Project1-Release\data\image\image\n01613177_3.JPEG", 2);
	int i;
	for(i = 0; i < colHis.dimNo; i++)
	{
		cout << colHis.his[i] << "  ";
	}*/
	 

IplImage *src;  



src = cvLoadImage("C:\/Users\/chenhz\/Desktop\/large_fQsQ_2b13000049931263.jpg");  

cvNamedWindow("bvin",CV_WINDOW_AUTOSIZE); 

cvShowImage("bvin",src); 


cvWaitKey(0);  

cvDestroyWindow("bvin"); 

cvReleaseImage(&src); 

	cout << "Press any key to continue..."<< endl;

  getchar(); // Wait for keypress on exit so we can read console output
}