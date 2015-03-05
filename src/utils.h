#ifndef SCHOONER_UTILS_H
#define SCHOONER_UTILS_H

#include <stdio.h>
#include <gdal.h>
#include <opencv2/opencv.hpp>

#define check(err, mess, args...) if(!(err)) { fprintf(stderr, mess, ##args); goto error; }

void
balance(std::vector<cv::Mat> &images, std::vector<cv::Mat> &dst){
  std::vector<std::vector<uint64_t> > hists(3);
  for(int i = 0; i < 3; i++)
    hists[i] = std::vector<uint64_t>(256, 0);

  std::vector<uint64_t> totals(3, 0);
  // calculate histogram across all files
  for(cv::Mat image : images){
    std::vector<cv::Mat> rgb;
    cv::split(image, rgb);

    for(int i = 0; i < rgb.size(); i++) {
      for(auto it = rgb[i].begin<uint8_t>(); it < rgb[i].end<uint8_t>(); it++){
        hists[i][*it]++;
        totals[i]++;
      }
    }
  }

  std::vector<std::pair<uint8_t, uint8_t> > minmax(3, std::make_pair(0,0));
  for(int i = 0; i < 3; i++){
    std::vector<uint64_t> hist = hists[i];
    uint64_t total = totals[i];
    uint8_t min = 0; uint64_t n = 0;
    while(hist[min] + n < total * 0.005)
      n += hist[min++];

    uint8_t max = 255; uint64_t x = 0;
    while(hist[max] + x < total * 0.005)
      x += hist[max--];

    minmax[i] = std::pair<uint8_t, uint8_t>(min, max);
  }

  for(cv::Mat image : images){
    std::vector<cv::Mat> rgb;
    cv::split(image, rgb);

    for(int i = 0; i < rgb.size(); i++) {
      std::pair<uint8_t, uint8_t> mm = minmax[i];
      float min = (float)mm.first;
      float max = (float)mm.second;
      rgb[i] = (rgb[i] - min) / (max - min) * 255 + min;
    }

    cv::Mat out;
    cv::merge(rgb, out);
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