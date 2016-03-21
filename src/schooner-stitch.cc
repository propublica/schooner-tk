#include <gdal_priv.h>
#include <iostream>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include <opencv2/stitching/detail/blenders.hpp>
#include <opencv2/stitching/detail/util.hpp>
#include "utils.h"

class Bounds {
 public:
  Bounds() {}

  int FromDataset(GDALDataset *ds) {
    double geotransform[6];
    CPLErr err = ds->GetGeoTransform(geotransform);
    check(err == CE_None, NULL);

    ul.first = geotransform[0];
    ul.second = geotransform[3];
    lr.first = ul.first + ds->GetRasterXSize() * geotransform[1];
    lr.second = ul.second + ds->GetRasterYSize() * geotransform[5];

    return false;
  error:
    return true;
  }

  void extend(Bounds obounds) {
    ul.first = std::min(obounds.ul.first, ul.first);
    ul.second = std::max(obounds.ul.second, ul.second);
    lr.first = std::max(obounds.lr.first, lr.first);
    lr.second = std::min(obounds.lr.second, lr.second);
  }

  std::pair<double, double> ul;
  std::pair<double, double> lr;
};

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cout << "usage: stitch <stitch-datasets*> out.tif" << std::endl;
    return -1;
  };

  // declarations
  std::vector<GDALDataset *> datasets;
  Bounds bounds;
  bool err;
  int j = 1;
  double geotransform[6];
  int xsize, ysize;
  cv::Ptr<cv::detail::Blender> blender =
      cv::detail::Blender::createDefault(cv::detail::Blender::FEATHER, false);
  std::vector<cv::Size> sizes;
  std::vector<cv::Point> corners;
  cv::Mat result, result_mask;
  GDALDataset *outds = NULL;
  cv::Size dst_sz;
  float blend_width;

  // init gdal
  GDALAllRegister();

  // open datasets
  for (int i = 1; i < argc - 1; i++) {
    GDALDataset *dataset = (GDALDataset *)GDALOpen(argv[i], GA_ReadOnly);
    check(dataset != NULL, "Could not open %s\n", argv[i]);
    datasets.push_back(dataset);
  }

  err = bounds.FromDataset(datasets.at(0));
  check(err == false, "%s doesn't have geo information", argv[1]);

  // fill out bookkeeping structures
  j = 1;
  for (GDALDataset *ds : datasets) {
    Bounds obounds;
    obounds.FromDataset(ds);
    check(err == false, "%s doesn't have geo information", argv[j]);
    bounds.extend(obounds);
    j++;
  }

  // no need to check err, we did that when we built the bounds up
  datasets[0]->GetGeoTransform(geotransform);
  xsize = (bounds.lr.first - bounds.ul.first) / geotransform[1];
  ysize = (bounds.lr.second - bounds.ul.second) / geotransform[5];
  printf("Creating dataset %s with size %d x %d \n", argv[argc - 1], xsize,
         ysize);
  printf("and bounds UL: %f %f LR: %f %f\n", bounds.ul.first, bounds.ul.second,
         bounds.lr.first, bounds.lr.second);

  geotransform[0] = bounds.ul.first;
  geotransform[2] = 0;
  geotransform[3] = bounds.ul.second;
  geotransform[4] = 0;

  // build sizes and point vectors
  j = 1;
  for (GDALDataset *ds : datasets) {
    Bounds obounds;
    obounds.FromDataset(ds);
    std::cout << ds->GetRasterXSize() << ", " << ds->GetRasterYSize()
              << std::endl;
    cv::Point pt(
        round((obounds.ul.first - geotransform[0]) / geotransform[1]),
        round((obounds.ul.second - geotransform[3]) / geotransform[5]));
    corners.push_back(pt);
    std::cout << "placing " << argv[j] << " at " << pt.x << ", " << pt.y
              << std::endl;

    cv::Size s(
        ((obounds.lr.first - geotransform[0]) / geotransform[1]) - pt.x,
        ((obounds.lr.second - geotransform[3]) / geotransform[5]) - pt.y);
    sizes.push_back(s);
    std::cout << "sizing " << argv[j] << " at " << s.width << ", " << s.height
              << std::endl;

    j++;
  }

  // from
  // https://github.com/Itseez/opencv/blob/0726c4d4ea80e73c96ccee7bd3ef5f71f46ac82b/samples/cpp/stitching_detailed.cpp#L799
  dst_sz = cv::detail::resultRoi(corners, sizes).size();
  blend_width = sqrt(static_cast<float>(dst_sz.area())) * 1 / 100.f;
  std::cout << "Blending sharpness set to " << 1.f / blend_width << std::endl;
  dynamic_cast<cv::detail::FeatherBlender *>(blender.get())
      ->setSharpness(1.f / blend_width);

  blender->prepare(corners, sizes);

  for (int i = 1; i < argc - 1; i++) {
    cv::Mat im = cv::imread(argv[i]);
    resize(im, im, sizes.at(i - 1), cv::INTER_LANCZOS4);
    std::vector<cv::Mat> channels;
    cv::split(im, channels);
    im.convertTo(im, CV_16SC3);
    std::cout << "merging " << argv[i] << std::endl;
    blender->feed(im, 0xFFFFFF & channels.at(0), corners.at(i - 1));
  }

  blender->blend(result, result_mask);
  std::cout << "writing " << argv[argc - 1] << std::endl;
  cv::imwrite(argv[argc - 1], result);
  std::cout << "done" << std::endl;

  outds = (GDALDataset *)GDALOpen(argv[argc - 1], GA_Update);
  check(outds != NULL, "Could not open %s\n", argv[argc - 1]);

  outds->SetGeoTransform(geotransform);
  outds->SetProjection(datasets.at(0)->GetProjectionRef());

  GDALClose(outds);
  for (GDALDataset *ds : datasets) GDALClose(ds);
  return 0;
error:
  if (outds) GDALClose(outds);
  for (GDALDataset *ds : datasets) GDALClose(ds);
  return -1;
}