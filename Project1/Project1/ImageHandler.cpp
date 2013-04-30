#include"ImageHandler.h"
#include<iostream>
#include<fstream>



 /*加载图像，将其交给特征提取器处理，然后输出特征*/
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
		 
		 fileIn.getline(buffer, 40);   //获取文件的名称，将其存在buffer中
		 fileName = buffer;       
		 
		 imagePath = fileDepositoryPath +"\/" + fileName;
		 IplImage* m_Image = cvLoadImage(imagePath.c_str(),CV_LOAD_IMAGE_COLOR);
		 //开始图像处理
		 feature = featureExtractor(m_Image, dim);  /*调用特征提取函数*/
		 //图像处理完毕
		 cvReleaseImage(&m_Image);

		 fileOut << "rect ";
		 for(w = 0; w < dim; w++)
			 fileOut << feature.data[w] << " ";
		 fileOut << endl;
		 delete feature.data;
		 cout << fileName << endl;   /*用于测试*/
	 }
	  fileOut<<"<< end >> ";
	  fileIn.close();
	  fileOut.close();
 }




 //根据文件名的形式获得一个文件名标签
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

 //自动地进行图像信息统计：文件分类统计/文件名列表加载到内存
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
		 fileIn.getline(buffer, 30);   //获取文件的名称，将其存在buffer中

		 fileName = buffer;              
		 imageList[imageNumber] = fileName;  //将文件名称添加到vector后面
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
		 //下面开始读取图像文件
		 //对文件特征进行处理
		 //将文件特征写到指定文件中*/
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


