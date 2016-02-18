#ifndef SCHOONER_UTILS_H
#define SCHOONER_UTILS_H

#include <assert.h>
#include <stdio.h>
#include <gdal.h>
#include <opencv2/opencv.hpp>

#define check(err, mess, args...)  \
  if (!(err)) {                    \
    fprintf(stderr, mess, ##args); \
    goto error;                    \
  }

// stretches histograms across multiple images
void balance(std::vector<cv::Mat> &images, std::vector<cv::Mat> &dst) {
  std::vector<std::pair<double, double> > minmax;

  for (cv::Mat image : images) {
    std::vector<cv::Mat> chans;
    cv::split(image, chans);
    int i = 0;
    for (cv::Mat chan : chans) {
      cv::Mat sorted(chan.reshape(0, 1).rows, chan.reshape(0, 1).cols,
                     chan.type());
      chan.reshape(0, 1).copyTo(sorted);
      cv::sort(sorted, sorted, cv::SORT_EVERY_ROW + cv::SORT_ASCENDING);

      if (minmax.size() < i + 1) minmax.push_back(std::make_pair(DBL_MAX, 0));

      int black_index = 0;
      while (sorted.at<uint16_t>(0, black_index) == 0 &&
             black_index < sorted.cols)
        black_index++;

      int white_index = sorted.cols;
      while (sorted.at<uint16_t>(0, white_index) == 65535 &&
             white_index > 0)
        white_index--;

      std::pair<double, double> &d = minmax.at(i);
      d.first = fmin(
          d.first,
          sorted.at<uint16_t>(
              0, (int)(white_index - black_index) * 0.01 / 100 + black_index));
      d.second = fmax(
          d.second,
          sorted.at<uint16_t>(
              0, (int)(white_index - black_index) * 99.9 / 100 + black_index));
      i++;
    }
  }

  for (cv::Mat image : images) {
    std::vector<cv::Mat> chans;
    cv::split(image, chans);

    for (int i = 0; i < chans.size(); i++) {
      std::pair<double, double> &d = minmax.at(i);
      chans[i] = (chans[i] - d.first) / (d.second - d.first) * 65535;
    }

    cv::Mat out;
    cv::merge(chans, out);
    dst.push_back(out);
  }
}

cv::Mat get_image(char *arg) {
  cv::Mat rgb = cv::imread(arg, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

  if (rgb.data == NULL) {
    std::cout << "couldn't read image: " << arg << std::endl;
    exit(1);
  }

  if (!(rgb.type() == CV_8U || rgb.type() == CV_8S || rgb.type() == CV_8SC1 ||
        rgb.type() == CV_8SC2 || rgb.type() == CV_8SC3 ||
        rgb.type() == CV_8SC4 || rgb.type() == CV_8UC1 ||
        rgb.type() == CV_8UC2 || rgb.type() == CV_8UC3 ||
        rgb.type() == CV_8UC4 || rgb.type() == CV_16U || rgb.type() == CV_16S ||
        rgb.type() == CV_16SC1 || rgb.type() == CV_16SC2 ||
        rgb.type() == CV_16SC3 || rgb.type() == CV_16SC4 ||
        rgb.type() == CV_16UC1 || rgb.type() == CV_16UC2 ||
        rgb.type() == CV_16UC3 || rgb.type() == CV_16UC4)) {
    std::cout << "schooner-contrast requires 8bit or 16bit images."
              << std::endl;
    exit(1);
  }

  if (rgb.type() == CV_8U || rgb.type() == CV_8S || rgb.type() == CV_8SC1 ||
      rgb.type() == CV_8SC2 || rgb.type() == CV_8SC3 || rgb.type() == CV_8SC4 ||
      rgb.type() == CV_8UC1 || rgb.type() == CV_8UC2 || rgb.type() == CV_8UC3 ||
      rgb.type() == CV_8UC4) {
    rgb.convertTo(rgb, CV_16U);
    rgb *= 256.0;
  }
  return rgb;
}

void assign_projection(GDALDatasetH src, GDALDatasetH dst) {
  double transform[6];
  if (GDALGetGeoTransform(src, transform) == CE_None)
    GDALSetGeoTransform(dst, transform);

  if (GDALGetProjectionRef(src) != NULL)
    GDALSetProjection(dst, GDALGetProjectionRef(src));
}

#endif