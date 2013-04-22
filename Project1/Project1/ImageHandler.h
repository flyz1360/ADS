#include"RTree.h"
#include<highgui.h>
#include<string>
#include<hash_map>
#include<vector>

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

	//ֱ��ͼ����ȡ�����
	ImageFeature extractColorHistogram(char* imagePath, int dimension); //��ȡ��ɫֱ��ͼ
	void exportRGBColorHistogramFile(char* imageListFile, string fileDepositoryPath, char* outFeatureFile, int dim);
	void exportLABColorHistogramFile(char* imageListFile, string fileDepositoryPath, char* outFeatureFile, int dim);
	
	ImageFeature parseImageFeature(string feature,int dim);

	
	
};