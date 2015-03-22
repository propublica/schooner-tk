#include "utils.h"

int
main(int argc, char **argv){
  cv::Mat ir, vis, res;
  GDALDatasetH src, out;
  check(argc == 3, "usage: schooner-ndvi <ir-band>.tif <red-band>.tif <ndvi>.tif");

  ir  = cv::imread(argv[1]);
  vis = cv::imread(argv[2]);

  res = (ir - vis) / (ir + vis + 0.00000001);

  cv::imwrite(argv[3], res);
  src = GDALOpen(argv[1], GA_ReadOnly);
  out = GDALOpen(argv[3], GA_Update);

  assign_projection(src, out);

  GDALClose(src);
  GDALClose(out);
  return 0;
error:
  return -1;
}