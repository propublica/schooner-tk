#include <iostream>
#include <gdal.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/core/types.hpp>
#include <opencv2/core/types_c.h>
#include <vector>
#include "utils.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cout << "usage: schooner-contrast src dst" << std::endl;
    exit(1);
  }

  cv::Mat rgb = get_image(argv[1]);

  std::vector<cv::Mat> images;
  images.push_back(rgb);
  std::vector<cv::Mat> out;

  balance(images, out);

  cv::Mat rgbc = out.at(0);
  rgbc /= 256.0;
  rgbc.convertTo(rgbc, CV_8U);
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

  assign_projection(gsrc, gdst);

  GDALClose(gsrc);
  GDALClose(gdst);
  return 0;
}