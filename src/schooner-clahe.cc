#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <gdal.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/core/types.hpp>
#include <opencv2/core/types_c.h>

// from opencv_contrib
namespace cv
{
namespace xphoto
{
    enum WhitebalanceTypes
    {
        /** perform smart histogram adjustments (ignoring 4% pixels with minimal and maximal
        values) for each channel */
        WHITE_BALANCE_SIMPLE = 0,
        WHITE_BALANCE_GRAYWORLD = 1
    };

    CV_EXPORTS_W void balanceWhite(const Mat &src, Mat &dst, const int algorithmType,
        const float inputMin  = 0.0f, const float inputMax  = 255.0f,
        const float outputMin = 0.0f, const float outputMax = 255.0f);
}
}

namespace cv
{
namespace xphoto
{

    template <typename T>
    void balanceWhite(std::vector < Mat_<T> > &src, Mat &dst,
        const float inputMin, const float inputMax,
        const float outputMin, const float outputMax, const int algorithmType)
    {
        switch ( algorithmType )
        {
            case WHITE_BALANCE_SIMPLE:
                {
                    /********************* Simple white balance *********************/
                    float s1 = 0.1f; // low quantile
                    float s2 = 0.1f; // high quantile

                    int depth = 2; // depth of histogram tree
                    if (src[0].depth() != CV_8U)
                        ++depth;
                    int bins = 16; // number of bins at each histogram level

                    int nElements = int( pow((float)bins, (float)depth) );
                     // number of elements in histogram tree

                    for (size_t i = 0; i < src.size(); ++i)
                    {
                        std::vector <int> hist(nElements, 0);

                        typename Mat_<T>::iterator beginIt = src[i].begin();
                        typename Mat_<T>::iterator endIt = src[i].end();

                        for (typename Mat_<T>::iterator it = beginIt; it != endIt; ++it)
                         // histogram filling
                        {
                            int pos = 0;
                            float minValue = inputMin - 0.5f;
                            float maxValue = inputMax + 0.5f;
                            T val = *it;

                            float interval = float(maxValue - minValue) / bins;

                            for (int j = 0; j < depth; ++j)
                            {
                                int currentBin = int( (val - minValue + 1e-4f) / interval );
                                ++hist[pos + currentBin];

                                pos = (pos + currentBin)*bins;

                                minValue = minValue + currentBin*interval;
                                maxValue = minValue + interval;

                                interval /= bins;
                            }
                        }

                        int total = int( src[i].total() );

                        int p1 = 0, p2 = bins - 1;
                        int n1 = 0, n2 = total;

                        float minValue = inputMin - 0.5f;
                        float maxValue = inputMax + 0.5f;

                        float interval = (maxValue - minValue) / float(bins);

                        for (int j = 0; j < depth; ++j)
                         // searching for s1 and s2
                        {
                            while (n1 + hist[p1] < s1 * total / 100.0f)
                            {
                                n1 += hist[p1++];
                                minValue += interval;
                            }
                            p1 *= bins;

                            while (n2 - hist[p2] > (100.0f - s2) * total / 100.0f)
                            {
                                n2 -= hist[p2--];
                                maxValue -= interval;
                            }
                            p2 = p2*bins - 1;

                            interval /= bins;
                        }

                        src[i] = (outputMax - outputMin) * (src[i] - minValue)
                            / (maxValue - minValue) + outputMin;
                    }
                    /****************************************************************/
                    break;
                }
            default:
                CV_Error_( CV_StsNotImplemented,
                    ("Unsupported algorithm type (=%d)", algorithmType) );
        }

        dst.create(/**/ src[0].size(), CV_MAKETYPE( src[0].depth(), int( src.size() ) ) /**/);
        cv::merge(src, dst);
    }

    /*!
    * Wrappers over different white balance algorithm
    *
    * \param src : source image (RGB)
    * \param dst : destination image
    *
    * \param inputMin : minimum input value
    * \param inputMax : maximum input value
    * \param outputMin : minimum output value
    * \param outputMax : maximum output value
    *
    * \param algorithmType : type of the algorithm to use
    */
    void balanceWhite(const Mat &src, Mat &dst, const int algorithmType,
        const float inputMin, const float inputMax,
        const float outputMin, const float outputMax)
    {
        switch ( src.depth() )
        {
            case CV_8U:
                {
                    std::vector < Mat_<uchar> > mv;
                    split(src, mv);
                    balanceWhite(mv, dst, inputMin, inputMax, outputMin, outputMax, algorithmType);
                    break;
                }
            case CV_16S:
                {
                    std::vector < Mat_<short> > mv;
                    split(src, mv);
                    balanceWhite(mv, dst, inputMin, inputMax, outputMin, outputMax, algorithmType);
                    break;
                }
            case CV_32S:
                {
                    std::vector < Mat_<int> > mv;
                    split(src, mv);
                    balanceWhite(mv, dst, inputMin, inputMax, outputMin, outputMax, algorithmType);
                    break;
                }
            case CV_32F:
                {
                    std::vector < Mat_<float> > mv;
                    split(src, mv);
                    balanceWhite(mv, dst, inputMin, inputMax, outputMin, outputMax, algorithmType);
                    break;
                }
            default:
                CV_Error_( CV_StsNotImplemented,
                    ("Unsupported source image format (=%d)", src.type()) );
                break;
        }
    }
}
}

int
main(int argc, char** argv) {
  if(argc != 3) {
    std::cout << "usage: schooner-clahe src dst" << std::endl;
    exit(1);
  }
  cv::Mat rgb = cv::imread(argv[1]);
  cv::Mat rgbc;
  cv::xphoto::balanceWhite(rgb, rgbc, cv::xphoto::WHITE_BALANCE_SIMPLE);

  cv::Mat lab;
  cv::cvtColor(rgbc, lab, cv::COLOR_RGB2Lab);

  std::vector<cv::Mat> lab_planes(3);
  cv::split(lab, lab_planes);

  cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
  clahe->setClipLimit(1);
  cv::Mat dst;
  clahe->apply(lab_planes[0], dst);

  dst.copyTo(lab_planes[0]);
  cv::merge(lab_planes, lab);

  cv::Mat clahe_img;
  cv::cvtColor(lab, clahe_img, cv::COLOR_Lab2RGB);
  cv::imwrite(argv[2], clahe_img);

  GDALAllRegister();
  GDALDatasetH gsrc = GDALOpen(argv[1], GA_ReadOnly);
  GDALDatasetH gdst = GDALOpen(argv[2], GA_Update);
  double transform[6];

  if(GDALGetGeoTransform(gsrc, transform) == CE_None)
    GDALSetGeoTransform(gdst, transform);

  if(GDALGetProjectionRef(gsrc) != NULL)
    GDALSetProjection(gdst, GDALGetProjectionRef(gsrc));

  GDALClose(gsrc);
  GDALClose(gdst);
  return 0;
}