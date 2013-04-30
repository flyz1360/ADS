#include"RTree.h"
#include<highgui.h>
#include<string>
#include<hash_map>
#include<vector>

#pragma once;
using namespace std;


//ͼ�����Ʒ����� ncatagory_index.JPG
struct ImageTag
{
	int catagory;           //��ʾͼ�������
	int index;              //ͼ�������µ��±�
};


struct ImageFeature
{
	int dimNo;
	double* data;
};

struct CPoint
{
	int x,y;
};


class ImageHandler{
	
public:
	string imageList[5613];

private:
	/*IplImage* m_Image;*/
	
	hash_map<int, int> imageInfo;   //ͳ��ͼ�����ͼ���Ϣ
	int imageNumber;                //��¼ͼ�������Ŀ
	

public: 
	
	ImageHandler(){ /*m_Image = NULL;*/imageNumber = 0; imageInfo = hash_map<int,int>();}

	ImageTag static parseImageName(string imageName);//�ļ�·����ȡͼƬ���������Ҹ���Ŀ¼�����ƽ��з���
	void inputImageInformation(char* imageListFilePath); 
	
	void exportFeatrueFile(ImageFeature featureExtractor(IplImage*,int),char* imageListFile, string fileDepositoryPath, char* outFeatureFile, int dim);
	
	ImageFeature parseImageFeature(string feature,int dim);
};