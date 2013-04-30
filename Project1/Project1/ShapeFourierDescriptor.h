#include"ImageHandler.h"

void grayImage(IplImage* m_Image);
void binarizeImage(IplImage* m_Image);
void extractImage(IplImage* Img1);
void trackImage(IplImage* Img1);

ImageFeature extractShapeFourierDescriptor(IplImage* Image,int dim);