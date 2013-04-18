#include"ImageHandler.h"
#include<iostream>
#include<fstream>


void ImageHandler::loadImage(char* imagePath)
{
	/*
	if(imagePath != NULL)
	{
		cvReleaseImage(&m_Image); 
		m_Image = cvLoadImage(imagePath,CV_LOAD_IMAGE_COLOR);
	}
	else
		m_Image = NULL;*/
}
/**
dimension:指的是RGB每个维度上的份数
*/
ImageFeature ImageHandler::extractColorHistogram(char* imagePath, int dim)
{
	ImageFeature colorHis;
	
	int dimension = dim * dim * dim;  //三个维度上均分
	colorHis.dimNo = dimension;
	colorHis.data = new double[dimension];
	int w, h, index;
	CvScalar s;
	for(w = 0; w < dimension; w++)
		colorHis.data[w] = 0.0;
	IplImage* m_Image = cvLoadImage(imagePath,CV_LOAD_IMAGE_COLOR);
	if(imagePath != NULL)
	{
		int width = m_Image->width;
		int height = m_Image->height;
		int r, g, b;
		double max = 1.0;
		int divider = 256/dim;
			
		for(h = 0; h < height; h++)
		{
			for(w = 0; w <width; w++)
			{
				s = cvGet2D(m_Image,h, w);

				index = ((int(s.val[0])) / divider + 1) *((int(s.val[1]))/divider + 1) * ((int(s.val[2]))/divider + 1); 
				if(index == dimension)
					index --;
				colorHis.data[index] += 1.0;
				if(colorHis.data[index] > max)
					max = colorHis.data[index];
			}
		}
		//进行归一化
		for(w = 0; w < dimension; w++)
			colorHis.data[w] /= max;
	}
	cvReleaseImage(&m_Image);

	return colorHis;
}

 void ImageHandler::exportColorHistogramFile(char* imageListFile, string fileDepositoryPath, char* outFeatureFile, int dim)
 {
	  ifstream fileIn(imageListFile, ios::in);
	  ofstream fileOut(outFeatureFile, ios::app);
	  fileOut << "tech nmos" << endl <<
			"<< polysilicon >>" << endl;

	  string fileName;
	  string imagePath;
	  
	  int width,w;
	  int height,h;
	  int r, g, b;
	  int dimension = dim * dim * dim;  //三个维度上均分
	  CvScalar s;
	  ImageFeature colorHis;
	  colorHis.dimNo = dimension;
	  colorHis.data = new double[dimension];
	  int divider = 256/dim;
	  int index;
	  double max;
	  double min;
	  
	  char buffer[30];
	  while(!fileIn.eof())
	  {
		 
		 fileIn.getline(buffer, 30);   //获取文件的名称，将其存在buffer中
		 fileName = buffer;       
		 
		 imagePath = fileDepositoryPath +"\/" + fileName;
		 IplImage* m_Image = cvLoadImage(imagePath.c_str(),CV_LOAD_IMAGE_COLOR);
		 //开始图像处理
		 width = m_Image->width;
		 height = m_Image->height;
		 for(w = 0; w < dimension; w++)
			colorHis.data[w] = 0.0;
		 
		 for(h = 0; h < height; h++)
		 {
			for(w = 0; w <width; w++)
			{
				s = cvGet2D(m_Image,h, w);

				index = ((int(s.val[0])) / divider + 1) *((int(s.val[1]))/divider + 1) * ((int(s.val[2]))/divider + 1)-1; 
				if(index == dimension)
					index --;
				colorHis.data[index] += 1.0;
			}
		 }
		 
		//进行归一化
		 max = 0.0;
		 min = 256.0;
		 for(w = 0; w < dimension; w++)
		 {
		 	if(colorHis.data[w] > max)
				max = colorHis.data[w];
			if(colorHis.data[w]< min)
				min = colorHis.data[w];
		 }
		 fileOut << "rect ";
		 for(w = 0; w < dimension; w++)
			 fileOut << (colorHis.data[w] - min) * 1000.0/(max-min) << " ";
			//fileOut << colorHis.data[w]/max << " ";
		 fileOut << endl;
		 //图像处理完毕
		 cvReleaseImage(&m_Image);
		 cout << fileName << endl;
	 }
	  fileOut<<"<< end >> ";
	  delete colorHis.data;
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
 void ImageHandler::inputImageInformation()
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

 //根据特征文件信息，建立指定类型的索引文件，并且返回一个索引结构
 //现在传参的时候需要进行修改
void ImageHandler::constructImageIndexFromFeatureFile(RTree<int,double,8,double> &rtree,char* inFeatureFilePath, int dim)
 {
	 ifstream fileIn(inFeatureFilePath, ios::in);
	 //ofstream fileOut("G:\/学业卷H\/大三下课程\/高级数据结构\/课程作业\/ADS-Project1-Release\/ADS-Project1-Release\/data\/color_feature_double.txt", ios::app);
	 int i = 0;
	 char buffer[60];
	 string feature;
	 ImageFeature imageFeature;
	
	 while(!fileIn.eof())
	 {
		 fileIn.getline(buffer,60);
		 feature = buffer;

		 if(feature[0] == 'r')
		 {
			 imageFeature = parseImageFeature(feature, dim);
			 /*
			 fileOut << "rect ";
			 for(int k = 0; k <9; k++)
			 {
				 fileOut << colorComment.data[k]/100.0 << " ";
			 }
			 fileOut << endl;*/
			 rtree.Insert(imageFeature.data, imageFeature.data,i);  //将数据添加到RTree中
			 i++;
		 }
		 
	 }
	 fileIn.close();
	 //fileOut.close();
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


