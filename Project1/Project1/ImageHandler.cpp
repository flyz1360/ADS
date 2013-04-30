#include"ImageHandler.h"
#include<iostream>
#include<fstream>



 /*����ͼ�񣬽��佻��������ȡ������Ȼ���������*/
 void ImageHandler::exportFeatrueFile(ImageFeature featureExtractor(IplImage*,int),char* imageListFile, string fileDepositoryPath, char* outFeatureFile, int dim)
 {
	  ifstream fileIn(imageListFile, ios::in);
	  ofstream fileOut(outFeatureFile, ios::app);
	  fileOut << "tech nmos" << endl <<
			"<< polysilicon >>" << endl;

	  string fileName;
	  string imagePath;
	  int w;
	  ImageFeature feature;
	  
	  char buffer[40];
	  while(!fileIn.eof())
	  {
		 
		 fileIn.getline(buffer, 40);   //��ȡ�ļ������ƣ��������buffer��
		 fileName = buffer;       
		 
		 imagePath = fileDepositoryPath +"\/" + fileName;
		 IplImage* m_Image = cvLoadImage(imagePath.c_str(),CV_LOAD_IMAGE_COLOR);
		 //��ʼͼ����
		 feature = featureExtractor(m_Image, dim);  /*����������ȡ����*/
		 //ͼ�������
		 cvReleaseImage(&m_Image);

		 fileOut << "rect ";
		 for(w = 0; w < dim; w++)
			 fileOut << feature.data[w] << " ";
		 fileOut << endl;
		 delete feature.data;
		 cout << fileName << endl;   /*���ڲ���*/
	 }
	  fileOut<<"<< end >> ";
	  fileIn.close();
	  fileOut.close();
 }




 //�����ļ�������ʽ���һ���ļ�����ǩ
 ImageTag ImageHandler::parseImageName(string imageName)
 {
	 ImageTag imageTag;
	 int index1 = imageName.find('_');  
	 int index2 = imageName.find('.');
	 string catagory = imageName.substr(1, index1 - 1);
	 string index = imageName.substr(index1 + 1, index2 -index1);
	 imageTag.catagory = atoi(catagory.c_str());
	 imageTag.index = atoi(index.c_str());
	 //cout << catagory  << "," << index << endl;
	 return imageTag;
 }

 //�Զ��ؽ���ͼ����Ϣͳ�ƣ��ļ�����ͳ��/�ļ����б���ص��ڴ�
 void ImageHandler::inputImageInformation(char* imageListFilePath)
 {

	 ifstream fileIn(imageListFilePath, ios::in);
     ImageTag imageTag;
	 
	 string fileName;
	 string imagePath;
	 hash_map<int, int>::iterator it;
	 while(!fileIn.eof())
	 {
		 char buffer[30];
		 fileIn.getline(buffer, 30);   //��ȡ�ļ������ƣ��������buffer��

		 fileName = buffer;              
		 imageList[imageNumber] = fileName;  //���ļ�������ӵ�vector����
		 imageTag = parseImageName(fileName);
		 imageInfo.find(imageTag.catagory);
		 it = imageInfo.find(imageTag.catagory);
		 if(it == imageInfo.end())
		 {
			 imageInfo[imageTag.catagory] = 1;
		 }
		 else
		 {
			 imageInfo[imageTag.catagory] += 1;
		 }
		 imageNumber++;

		 /*
		 imagePath = fileDepositoryPath + '/' + fileName; 
		 //���濪ʼ��ȡͼ���ļ�
		 //���ļ��������д���
		 //���ļ�����д��ָ���ļ���*/
	 }

	 fileIn.close();
 }

ImageFeature ImageHandler::parseImageFeature(string feature,int dim)
{
	ImageFeature imageFeature;
	int index, i;
	imageFeature.dimNo = dim;
	imageFeature.data = new double[dim];
	feature = feature.substr(5);
	for(i = 0; i < dim-1; i++)
	{
		index = feature.find(' ');
		imageFeature.data[i] = atof(feature.substr(0, index).c_str());
		feature = feature.substr(index + 1);
	}
	imageFeature.data[dim-1] = atoi(feature.c_str());// / 10000.0;
	return imageFeature;
}


