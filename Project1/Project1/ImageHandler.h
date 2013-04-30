#include"RTree.h"
#include<highgui.h>
#include<string>
#include<hash_map>
#include<vector>

#pragma once;
using namespace std;


//图像名称方法： ncatagory_index.JPG
struct ImageTag
{
	int catagory;           //表示图像的种类
	int index;              //图像种类下的下标
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
	
	hash_map<int, int> imageInfo;   //统计图像类型及信息
	int imageNumber;                //记录图像的总数目
	

public: 
	
	ImageHandler(){ /*m_Image = NULL;*/imageNumber = 0; imageInfo = hash_map<int,int>();}

	ImageTag static parseImageName(string imageName);//文件路径提取图片特征，并且根据目录和名称进行分类
	void inputImageInformation(char* imageListFilePath); 
	
	void exportFeatrueFile(ImageFeature featureExtractor(IplImage*,int),char* imageListFile, string fileDepositoryPath, char* outFeatureFile, int dim);
	
	ImageFeature parseImageFeature(string feature,int dim);
};