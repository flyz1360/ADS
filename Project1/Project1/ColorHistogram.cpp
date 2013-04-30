#include"ColorHistogram.h"


/*提取颜色直方图的特征*/
ImageFeature extractColorHistogram(IplImage* m_Image, int dim)
{
	int width,w;
	int height,h;
	int ri, gi, bi;
	int dimension = dim * dim * dim;  //三个维度上均分
	ImageFeature colorHis;
	colorHis.dimNo = dimension;
	colorHis.data = new double[dimension];
	CvScalar s;
	double divider = 256/dim + 1;
	int index;
	double max;
	double min;
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
			ri = (int)floor(s.val[0] / divider);
			gi = (int)floor(s.val[1] / divider);
			bi = (int)floor(s.val[2] / divider);
			index = ri * dim * dim + gi * dim  + bi; 
			if(index == dimension)
				index --;
			colorHis.data[index] += 1.0;
		}
	}
		 
	//进行归一化
		 
	max = 0.0;
	min = (numeric_limits<double>::max)();
	for(w = 0; w < dimension; w++)
	{
	if(colorHis.data[w] > max)
		max = colorHis.data[w];
	if(colorHis.data[w]< min)
		min = colorHis.data[w];
	}
	for(w = 0; w < dimension; w++)
		colorHis.data[w]= (colorHis.data[w] - min) * 1000.0/(max-min);

	return colorHis;
}