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
 ColorHistogram ImageHandler::extractColorHostogram(char* imagePath, int dim)
{
	ColorHistogram colorHis;
	/*
	int dimension = dim * dim * dim;  //三个维度上均分
	colorHis.dimNo = dimension;
	int w, h, index;
	CvScalar s;
	for(w = 0; w < dimension; w++)
		colorHis.his[w] = 0.0;

	if(dimension == 9)
	{
		loadImage(imagePath);
		if(imagePath != NULL)
		{
			int width = m_Image->width;
			int height = m_Image->height;
			
			for(h = 0; h < height; h++)
			{
				for(w = 0; w <width; w++)
				{
					s = cvGet2D(m_Image,h, w);
					index = ((int)s.val[0] / dim) *((int)s.val[1]/dim) * ((int)s.val[2]/dim); 
					if(index == dimension)
						index --;
					colorHis.his[index]++;
				}
			}
		}
	}*/
	return colorHis;
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
void ImageHandler::constructImageIndexFromFeatureFile(RTree<int,double,9,double> &rtree,char* inFeatureFilePath)
 {
	 ifstream fileIn(inFeatureFilePath, ios::in);
	 //ofstream fileOut("G:\/学业卷H\/大三下课程\/高级数据结构\/课程作业\/ADS-Project1-Release\/ADS-Project1-Release\/data\/color_feature_double.txt", ios::app);
	 int i = 0;
	 char buffer[60];
	 string feature;
	 ColorComment colorComment;
	
	 while(!fileIn.eof())
	 {
		 fileIn.getline(buffer,60);
		 feature = buffer;

		 if(feature[0] == 'r')
		 {
			 colorComment = parserImageCommentFeature(feature);
			 /*
			 fileOut << "rect ";
			 for(int k = 0; k <9; k++)
			 {
				 fileOut << colorComment.data[k]/100.0 << " ";
			 }
			 fileOut << endl;*/
			 rtree.Insert(colorComment.data, colorComment.data,i);  //将数据添加到RTree中
			 i++;
		 }
		 
	 }
	 fileIn.close();
	 //fileOut.close();
}
ColorComment ImageHandler::parserImageCommentFeature(string feature)
{
	ColorComment colorComment;
	int index, i;
	feature = feature.substr(5);
	for(i = 0; i < 8; i++)
	{
		index = feature.find(' ');
		colorComment.data[i] = atof(feature.substr(0, index).c_str());
		feature = feature.substr(index + 1);
	}
	colorComment.data[8] = atoi(feature.c_str());// / 10000.0;
	return colorComment;
}


