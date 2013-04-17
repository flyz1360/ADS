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
	
	/*Test Commit*/
	ImageHandler imageHandler = ImageHandler();
	int dimension = 2;
	/*
	ColorHistogram colorHis = imageHandler.extractColorHistogram("G:\/ѧҵ��H\/�����¿γ�\/�߼����ݽṹ\/�γ���ҵ\/ADS-Project1-Release\/ADS-Project1-Release\/data\/image\/image\/n01613177_3.JPEG",dimension);
	for(int k = 0; k < colorHis.dimNo; k++)
	{
		cout << colorHis.his[k] << ", ";
	}
	cout << endl;*/
	RTree<int,double,8,double> rtree;
	char* imageListFile = "G:\/ѧҵ��H\/�����¿γ�\/�߼����ݽṹ\/�γ���ҵ\/ADS-Project1-Release\/ADS-Project1-Release\/data\/imagelist.txt";
	char* fileDepositoryPath = "G:\/ѧҵ��H\/�����¿γ�\/�߼����ݽṹ\/�γ���ҵ\/ADS-Project1-Release\/ADS-Project1-Release\/data\/image\/image";
	imageHandler.setImageListFilePath(imageListFile);
	imageHandler.inputImageInformation();
	char* outFeatureFile= "G:\/ѧҵ��H\/�����¿γ�\/�߼����ݽṹ\/�γ���ҵ\/ADS-Project1-Release\/ADS-Project1-Release\/data\/color_his1000.txt";
	//imageHandler.exportColorHistogramFile(imageListFile,fileDepositoryPath,outFeatureFile, dimension);
	
	imageHandler.constructImageIndexFromFeatureFile(rtree,outFeatureFile,8);
	int k = 10;
	
	//double feature[8] ={523.751, 415.287, 0, 144.897, 0, 0, 0, 1000 };
	double feature[8] ={675.969, 598.488, 0, 566.323, 0, 0, 0, 1000  };
	//rtree.Search(feature,feature,MySearchCallback,NULL);

	
	int* nnList = rtree.KNN(feature,k);

	string* nameList = new string[k];
	int i;
	for(i = 0; i <k; i++)
	{
		nameList[i] = imageHandler.imageList[nnList[i]];
		cout << i<<": "<< nameList[i] << endl;
	}
	










	/*
	RTree<int,double,9,double> rtree;  //string��ʾͼ������ݱ�ʾΪһ��string
	imageHandler.setImageListFilePath(imageListFile);
	imageHandler.inputImageInformation();
	imageHandler.constructImageIndexFromFeatureFile(rtree,"G:\/ѧҵ��H\/�����¿γ�\/�߼����ݽṹ\/�γ���ҵ\/ADS-Project1-Release\/ADS-Project1-Release\/data\/color_feature.txt");
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
	ColorHistogram colHis = imageHandler.extractColorHostogram("G:\ѧҵ��H\�����¿γ�\�߼����ݽṹ\�γ���ҵ\ADS-Project1-Release\ADS-Project1-Release\data\image\image\n01613177_3.JPEG", 2);
	int i;
	for(i = 0; i < colHis.dimNo; i++)
	{
		cout << colHis.his[i] << "  ";
	}*/
	/*
	 IplImage *src; // ����IplImageָ�����src   
     
     src = cvLoadImage("lena.jpg",-1); // ��srcָ��ǰ�����ļ�Ŀ¼�µ�ͼ��lena.jpg  
     cvNamedWindow("lena",0);//����һ��������Ϊlena����ʾ����  
     cvShowImage("lena",src);//��lena�����У���ʾsrcָ����ָ���ͼ��  
     cvWaitKey(0);//���޵ȴ�����ͼ������ʾ  
     cvDestroyWindow("lena");//���ٴ���lena  
     cvReleaseImage(&src);//�ͷ�IplImageָ��src  */

  cout << "Press any key to continue..."<< endl;
  getchar(); // Wait for keypress on exit so we can read console output
}