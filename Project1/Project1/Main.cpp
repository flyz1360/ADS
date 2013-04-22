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
	
	/**1.ͼ���ļ���صĴ���*/
	ImageHandler imageHandler = ImageHandler();  //��ʼ��ͼ������
	      //������ͼ���ļ��洢·��
	char* imageListFile = "G:\/ѧҵ��H\/�����¿γ�\/�߼����ݽṹ\/�γ���ҵ\/ADS-Project1-Release\/ADS-Project1-Release\/data\/imagelist.txt";
	char* fileDepositoryPath = "G:\/ѧҵ��H\/�����¿γ�\/�߼����ݽṹ\/�γ���ҵ\/ADS-Project1-Release\/ADS-Project1-Release\/data\/image\/image";
	     //����ȡ���������������ļ���
	char* outFeatureFile= "G:\/ѧҵ��H\/�����¿γ�\/�߼����ݽṹ\/�γ���ҵ\/ADS-Project1-Release\/ADS-Project1-Release\/data\/color_his1000.txt";
	
	/**2.��ȡ������Ϣ*/
	/*imageHandler.exportColorHistogramFile(imageListFile,fileDepositoryPath,outFeatureFile,dimension);*/

	int dimension = 2;//��ɫֱ��ͼ��ά�� 2*2*2 = 8
	
	imageHandler.setImageListFilePath(imageListFile);
	imageHandler.inputImageInformation();//����ͼ����Ϣ
	

	/**3.��������*/
	RTree<int,double,8,double> rtree;
	imageHandler.constructImageIndexFromFeatureFile(rtree,outFeatureFile,8);  //��ȡ�����ļ����ݣ���������


	/**4.k������ȡ*/
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