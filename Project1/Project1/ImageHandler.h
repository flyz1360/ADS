#include"RTree.h"
#include<highgui.h>
#include<string>
#include<hash_map>
#include<vector>

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






class ImageHandler{
	
public:
	string imageList[5613];

private:
	/*IplImage* m_Image;*/
	
	hash_map<int, int> imageInfo;   //统计图像类型及信息
	int imageNumber;                //记录图像的总数目
	char *fileDepositoryPath;       //图像的储藏路径
	char* imageListFilePath;        //文件名列表文件路径
	
	
	void loadImage(char* imagePath); //

public: 
	
	ImageHandler(){ /*m_Image = NULL;*/imageNumber = 0; imageInfo = hash_map<int,int>();}
	void setFileDepositoryPath(char* fileDepositoryPath ){this->fileDepositoryPath = fileDepositoryPath;}
	void setImageListFilePath(char* imageListFilePath){this->imageListFilePath  = imageListFilePath;}

	ImageTag parseImageName(string imageName);//文件路径提取图片特征，并且根据目录和名称进行分类
	void inputImageInformation(); 

	//直方图的提取和输出
	ImageFeature extractColorHistogram(char* imagePath, int dimension); //提取颜色直方图
	void exportColorHistogramFile(char* imageListFile, string fileDepositoryPath, char* outFeatureFile, int dim);

	
	ImageFeature parseImageFeature(string feature,int dim);

	
	void constructImageIndexFromFeatureFile(RTree<int,double,8,double> &rtree,char* inFeatureFilePath, int dim);
};