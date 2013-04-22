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

void constructImageIndexFromFeatureFile(RTree<int,double,27,double> rtree ,char* inFeatureFilePath, int dim)
{
	 ifstream fileIn(inFeatureFilePath, ios::in);
	 //ofstream fileOut("G:\/ѧҵ��H\/�����¿γ�\/�߼����ݽṹ\/�γ���ҵ\/ADS-Project1-Release\/ADS-Project1-Release\/data\/color_feature_double.txt", ios::app);
	 int i = 0;
	 char buffer[60];
	 string feature;
	 ImageFeature imageFeature;
	 ImageHandler imageHandler;
	
	 while(!fileIn.eof())
	 {
		 fileIn.getline(buffer,60);
		 feature = buffer;

		 if(feature[0] == 'r')
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
	/**1.ͼ���ļ���صĴ���*/
	ImageHandler imageHandler = ImageHandler();  //��ʼ��ͼ������
	      //������ͼ���ļ��洢·��
	char* imageListFile = "G:\/ѧҵ��H\/�����¿γ�\/�߼����ݽṹ\/�γ���ҵ\/ADS-Project1-Release\/ADS-Project1-Release\/data\/imagelist.txt";
	char* fileDepositoryPath = "G:\/ѧҵ��H\/�����¿γ�\/�߼����ݽṹ\/�γ���ҵ\/ADS-Project1-Release\/ADS-Project1-Release\/data\/image\/image";
	     //����ȡ���������������ļ���
	char* outFeatureFile= "G:\/ѧҵ��H\/�����¿γ�\/�߼����ݽṹ\/�γ���ҵ\/ADS-Project1-Release\/ADS-Project1-Release\/data\/color_dim2_norm1000.txt";
	


	int dimension = 2;//��ɫֱ��ͼ��ά��
	
	imageHandler.inputImageInformation(imageListFile);//����ͼ����Ϣ

	/**2.��ȡ������Ϣ*/
	imageHandler.exportRGBColorHistogramFile(imageListFile,fileDepositoryPath,outFeatureFile,dimension);

	

	/**3.��������*/
	/*rtreeģ�潫ά�Ȱ�����ȥ�ˣ����ÿ�α任ζ����ʱ�򣬾ȵð�r�������޸�һ�飬�Һ��潫ά�ȴ�ģ���������������ʹ��ɶ�ָ̬��*/
	
	/*RTree<int,double,27,double> rtree;
	constructImageIndexFromFeatureFile(rtree,outFeatureFile,27);*/  //��ȡ�����ļ����ݣ���������


	/**4.k������ȡ*/
	/*
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
	*/

    cout << "Press any key to continue..."<< endl;
    getchar(); // Wait for keypress on exit so we can read console output
}