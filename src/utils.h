#ifndef SCHOONER_UTILS_H
#define SCHOONER_UTILS_H
#include <assert.h>
#include <stdio.h>
#include <gdal.h>
#include <opencv2/opencv.hpp>

#define check(err, mess, args...) if(!(err)) { fprintf(stderr, mess, ##args); goto error; }

// stretches histograms across multiple images
void
balance(std::vector<cv::Mat> &images, std::vector<cv::Mat> &dst){
  std::vector<std::pair<double, double> > minmax;

  for(cv::Mat image : images){
    std::vector<cv::Mat> chans;
    cv::split(image, chans);
    int i = 0;
    for(cv::Mat chan : chans) {
      cv::Mat sorted(chan.reshape(0,1).rows, chan.reshape(0,1).cols, chan.type());
      chan.reshape(0,1).copyTo(sorted);
      std::cout << sorted.size().width << "," << sorted.size().height << std::endl;
      cv::sort(sorted, sorted, cv::SORT_EVERY_ROW + cv::SORT_ASCENDING);
      double min, max;
      cv::minMaxLoc(sorted, &min, &max);

      if(minmax.size() < i + 1)
        minmax.push_back(std::make_pair(DBL_MAX, 0));

      int black_index = 0;
      while(sorted.at<uint16_t>(0, black_index) == 0 && black_index < sorted.cols) black_index++;

      std::pair<double, double> &d = minmax.at(i);
      d.first  = fmin(d.first,  sorted.at<uint16_t>(0,(int)(sorted.cols - black_index) * 0.1 / 100 + black_index));
      d.second = fmax(d.second, sorted.at<uint16_t>(0,(int)(sorted.cols - black_index) * 99.9 / 100 + black_index));
      i++;
    }
  }

  for(cv::Mat image : images){
    std::vector<cv::Mat> chans;
    cv::split(image, chans);

    for(int i = 0; i < chans.size(); i++) {
      std::pair<double, double> &d = minmax.at(i);
      std::cout << d.first << ", " << d.second << std::endl;
      chans[i] = (chans[i] - d.first) / (d.second - d.first) * 65535;
    }

    cv::Mat out;
    cv::merge(chans, out);
    dst.push_back(out);
  }
}




void
assign_projection(GDALDatasetH src, GDALDatasetH dst){
  double transform[6];
  if(GDALGetGeoTransform(src, transform) == CE_None)
    GDALSetGeoTransform(dst, transform);

  if(GDALGetProjectionRef(src) != NULL)
    GDALSetProjection(dst, GDALGetProjectionRef(src));
}

#endif