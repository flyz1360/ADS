#include"ShapeFourierDescriptor.h"
#include<iostream>
#include<fstream>
using namespace std;
#define PI 3.141592653


/*���лҶȻ�����*/
void grayImage(IplImage* m_Image)
{
	// TODO: Add your control notification handler code here
	int width=m_Image->width;
	int height=m_Image->height;
	int h ,w;
	CvScalar s;
	for(h = 0; h < height; h++)
	{
		for(w = 0; w <width; w++)
		{
			s = cvGet2D(m_Image,h, w);
			unsigned char grayindex = (unsigned char)(0.114*s.val[0]+ 0.587*s.val[1] + 0.299*s.val[2]);
			s.val[0] = grayindex;
			s.val[1] = grayindex;
			s.val[2] = grayindex;
			cvSet2D(m_Image,h,w,s);
		}
	}
}


/*���ж�ֵ������*/
void binarizeImage(IplImage* m_Image)
{
	// TODO: Add your control notification handler code here
	int i,j;
	int grayindex;
	int width=m_Image->width;
	int height=m_Image->height;
	int pixelindex[256];
	CvScalar s;
	for(int x=0;x<256;x++)
	{
		pixelindex[x]=0;
	}
	for(j=0;j<height;j++)
	{
		for(i=0;i<width;i++)
		{
			s = cvGet2D(m_Image,j, i);
			grayindex=s.val[0];
			pixelindex[grayindex]++;
		}	
	}

	int count0,sum=0;
	int T,BestT;
	float u0,u1;
	float w0,w1;
	float variance,Maxvariance=0;
	/*���㷽��*/
	for(i=0;i<256;i++)
	{
		sum += pixelindex[i];
	}

	for(T=0;T<256;T++)
	{
		u0=0;
		count0=0;
		for(i=0;i<=T;i++)
		{
			u0 += i*pixelindex[i];
			count0 += pixelindex[i];
		}
		u0 /= count0;
		w0=(float)count0/sum;

		u1=0;
		for(i=T+1;i<256;i++)
		{
			u1 += i*pixelindex[i];				
		}
		u1 /= (sum-count0);
		w1=1-w0;
		variance=w0*w1*(u0-u1)*(u0-u1);
		if(variance>Maxvariance)
		{
			Maxvariance=variance;
			BestT=T;
		}	
	}

	/*���ж�ֵ��*/
	for(j=0;j<height;j++)
	{
		for(i=0;i<width;i++)
		{
			s = cvGet2D(m_Image,j, i);
			if(s.val[0]<BestT)
			{
				s.val[0]=255;
				s.val[1]=255;
				s.val[2]=255;
				cvSet2D(m_Image,j,i,s);
			
			}
			else
			{
				s.val[0]=0;
				s.val[1]=0;
				s.val[2]=0;
				cvSet2D(m_Image,j,i,s);
			}		
		}	
	}
}


/*�߽���ȡ������8���ڵķ���*/
void extractImage(IplImage* Img1)
{
	int i,j;	
	int width=Img1->width;
	int height=Img1->height;
	CvScalar s1,s2;
	IplImage* Img2 = cvCreateImage(cvGetSize(Img1), Img1->depth,Img1->nChannels);
    cvCopy(Img1, Img2, NULL);

	for(j=0;j<height;j++)  /*�տ�ʼ��ͼ������Ϊ��ɫ*/
	{
		for(i=0;i<width;i++)
		{
			s2 = cvGet2D(Img2,j, i);
			s2.val[0]=255;
			s2.val[1]=255;
			s2.val[2]=255;
			cvSet2D(Img2,j,i,s2);
		}
	}
	

	for(j=1;j<height-1;j++)
	{
		for(i=1;i<width-1;i++)
		{
			s1 = cvGet2D(Img1,j, i);
			if(s1.val[0]==0)
			{
				s2 = cvGet2D(Img2,j, i);
				s2.val[0]=0;
				s2.val[1]=0;
				s2.val[2]=0;
				cvSet2D(Img2,j,i,s2);
				int temp = cvGet2D(Img1,j-1, i-1).val[0]
						 + cvGet2D(Img1,j-1, i).val[0]
						 + cvGet2D(Img1,j-1, i+1).val[0]
						 + cvGet2D(Img1,j, i-1).val[0]
						 + cvGet2D(Img1,j, i+1).val[0]
						 + cvGet2D(Img1,j+1, i-1).val[0]
						 + cvGet2D(Img1,j+1, i).val[0]
						 + cvGet2D(Img1,j+1, i+1).val[0];
				
				if(temp==0)
				{
					s2.val[0]=255;
					s2.val[1]=255;
					s2.val[2]=255;		
					cvSet2D(Img2,j,i,s2);
				}			
			}
		}						
	}	

	/*��ͼ��ת�������д���*/
	cvCopy(Img2,Img1, NULL);
	cvReleaseImage(&Img2);
}



void drawBlock(IplImage* Img, int i, int j, int w, int h)
{
	CvScalar s2;
	cout << "block" << endl;
	int Direction[8][2]={{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}};
	for(int k = 0; k < 8; k++)
	{
		if(i + Direction[k][0] < w && j + Direction[k][1] < h)
		{
			s2 = cvGet2D(Img,j + Direction[k][1], i + Direction[k][0]);
			s2.val[0]=0;
			s2.val[1]=255;
			s2.val[2]=0;
			cvSet2D(Img,j + Direction[k][1], i + Direction[k][0],s2);
		}
	}
}


/*�߽����*/
void trackImage(IplImage* Img1)
{
	ofstream fileOut("C://Users//chenhz//Desktop//edge.txt", ios::app);
	int i,j;	
	int grayindex;
	int width=Img1->width;
	int height=Img1->height;
	CvScalar s1,s2;
	IplImage* Img2 = cvCreateImage(cvGetSize(Img1), Img1->depth,Img1->nChannels);
    cvCopy(Img1, Img2, NULL);

	bool FindStartPoint;
	bool FindPoint;
	CPoint StartPoint,CurrentPoint;

	int Direction[8][2]={{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0}};
	int BeginDirect;

	for(j=0;j<height;j++)  /*�տ�ʼ��ͼ������Ϊ��ɫ*/
	{
		for(i=0;i<width;i++)
		{
			s2 = cvGet2D(Img2,j, i);
			s2.val[0]=255;
			s2.val[1]=255;
			s2.val[2]=255;
			cvSet2D(Img2,j,i,s2);
		}
	}

	FindStartPoint=false;	
	/*Ѱ�ҵ��߽����ʼ��*/
	for(j=0;j<height && !FindStartPoint;j++)
	{
		for(i=0;i<width && !FindStartPoint;i++)
		{
			s1 = cvGet2D(Img1,j, i);
			if(s1.val[0]==0)
			{
				FindStartPoint=true;
				StartPoint.y=j;
				StartPoint.x=i;
				s2 = cvGet2D(Img2,j, i);
				s2.val[0]=0;
				s2.val[1]=0;
				s2.val[2]=0;	
				cvSet2D(Img2,j,i,s2);
			}
		}						

	}	

	BeginDirect=0;
	FindStartPoint=false;
	CurrentPoint.y=StartPoint.y;
	CurrentPoint.x=StartPoint.x;
	int count = 0;

	/*���߽磬֪��������ʼ��*/
	while(!FindStartPoint)  /*�������ѭ���Ϳ��Եõ���Ҫ�߽�����*/
	{
		FindPoint=false;
		while(!FindPoint)
		{
			s1 = cvGet2D(Img1,(CurrentPoint.y+Direction[BeginDirect][1]), (CurrentPoint.x+Direction[BeginDirect][0]));
			grayindex=s1.val[0];

			if(grayindex==0)
			{
				FindPoint=true;
				CurrentPoint.y=CurrentPoint.y+Direction[BeginDirect][1];
				CurrentPoint.x=CurrentPoint.x+Direction[BeginDirect][0];

				if(CurrentPoint.y==StartPoint.y && CurrentPoint.x==StartPoint.x)
				{
					FindStartPoint=true;
				}
				else 
				{
					s2 = cvGet2D(Img2,CurrentPoint.y, CurrentPoint.x);
					fileOut << CurrentPoint.x << "," << CurrentPoint.y << "  ";
					fileOut << count ++ << endl;
					s2.val[0]=0;
					s2.val[1]=0;
					s2.val[2]=0;	
					cvSet2D(Img2,j,i,s2);
					drawBlock(Img2, CurrentPoint.x, CurrentPoint.y,  width, height);
					BeginDirect -= 2;
						
					if(BeginDirect<0)
					{
						BeginDirect += 8;				
					}
				}
			}
			else
			{
				BeginDirect++;
				if(BeginDirect==8)
				{
					BeginDirect=0;
				}			
			}	
		}
	}
	fileOut.close();
	/*��ͼ��ת�������д���*/
	cvCopy(Img2,Img1, NULL);
	cvReleaseImage(&Img2);
}



/*��ȡ����Ҷ������*/

ImageFeature Fourierdescriber(IplImage* Img, int dim)
{

	ImageFeature feature;
	feature.dimNo = dim;
	feature.data = new double[dim];
	int j = 0, i;
	int width=Img->width;
	int height=Img->height;
	CvScalar s;
	CPoint *newP = new CPoint[width * height]; 
	int n = 0;
	for(j = 0; j < height; j++)  /*���߽����ȡ����*/
	{
		for(i=0;i<width;i++)
		{
			s = cvGet2D(Img,j, i);
			if(s.val[0] == 0)
			{
				newP[n].x = i;
				newP[n].y = j;
				n ++;
			}
		}
	}
	
	complex<double> * Z=new complex<double>[n];//���帵��Ҷ�����ӣ��������ͣ� 
	memset(Z,(0.0,0.0),j); 
	/*
	for(j = 0; j < n; j++)
	{
		Z[j]. real = 0.0;
		Z[j].imag = 0.0;
	}*/
	complex<double> temp=(0.0,0.0);   
	double *d=new double[n];
	
	for(int k=0;k<=dim;k++)//ǰdim��������   1,2,3,..., dim
	{   
		complex<double> temp=(0.0, 0.0);   
		for(int l=0;l<n;l++)   
		{      
			temp=temp+complex<double>(newP[l].x, newP[l].y)*complex<double>(cos((2*PI*l*k)/n),-sin((2*PI*l*k)/n));   
		}   
		Z[k]=temp/complex<double>((double)n,0.0);   
		//��ͳ�Ĺ�һ������Ҷ������    
		if (k>0)
		{
			d[k]=sqrt(pow(Z[k].real(),2)+pow(Z[k].imag(),2))/sqrt(pow(Z[0].real(),2)+pow(Z[0].imag(),2)); 
			feature.data[k-1] = d[k];
		}	
	}

		
	//	fprintf(fileHandle, "\r\n");
	delete[] newP ;
	delete[] Z;
	delete[] d;
	
	return feature;
}




ImageFeature extractShapeFourierDescriptor(IplImage* Image,int dim)
{
	ImageFeature shape;
	shape.dimNo = dim;
	/*����������ȡ*/

    grayImage(Image); 
	cout << "gray done" << endl;	
	binarizeImage(Image);
	cout << "binary done" << endl;


    extractImage(Image);
		cout << "extract done" << endl;


	shape = Fourierdescriber(Image,dim);
	
					/*
			cvNamedWindow("mainWin", CV_WINDOW_AUTOSIZE); //���崰��
    cvShowImage("mainWin", Image );   //�ڴ�����ʾ
	cvWaitKey(0);
    cvDestroyWindow("mainWin");					

    trackImage(Image);
	cout << "track done" << endl;*/



	return shape;
}
