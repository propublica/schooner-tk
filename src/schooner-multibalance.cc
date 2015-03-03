#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <gdal.h>
#include "utils.h"

int
main(int argc, char** argv) {
  std::vector<cv::Mat> images;
  for(int i = 1; i < argc; i++){
    cv::Mat rgb = cv::imread(argv[i]);
    images.push_back(rgb);
  }
  std::vector<cv::Mat> dst;

  balance(images, dst);

  GDALAllRegister();

  for(int i = 1; i < argc; i++){
    std::string out(argv[i]);
    out.append(".balanced.tif");
    std::cout << "writing " << out << std::endl;
    cv::imwrite(out, dst[i - 1]);

    GDALDatasetH gsrc = GDALOpen(argv[i], GA_ReadOnly);
    GDALDatasetH gdst = GDALOpen(out.c_str(), GA_Update);

    assign_projection(gsrc, gdst);

    GDALClose(gsrc);
    GDALClose(gdst);
  }
}