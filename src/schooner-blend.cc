#include <cpl_error.h>
#include <cpl_conv.h>
#include <cpl_string.h>
#include <gdal.h>
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

const char *usage = "usage: schooner-blend <blend-datasets*> out.tif";

void close_datasets(GDALDatasetH *datasets, int num) {
  for (int i = 0; i < num; i++)
    if (datasets[i] != NULL) {
      GDALClose(datasets[i]);
      datasets[i] = NULL;
    }
}

bool open_datasets(GDALDatasetH *datasets, int num, char **files) {
  for (int i = 0; i < num; i++) {
    int (*checks[])(GDALDatasetH) = {GDALGetRasterCount, GDALGetRasterXSize,
                                     GDALGetRasterYSize};
    int j;
    datasets[i] = GDALOpen(files[i], GA_ReadOnly);
    check(datasets[i] != NULL, "Couldn't open %s\n%s", files[i],
          CPLGetLastErrorMsg());

    for (j = 0; j < 3; j++)
      check(checks[j](datasets[0]) == checks[j](datasets[i]),
            "%s not the same size as %s (%i vs %i)\n", files[0], files[i],
            checks[j](datasets[0]), checks[j](datasets[i]));
  }

  return true;
error:
  return false;
}

int sort_int(const void *a, const void *b) {
  return (*(uint8_t *)a > *(uint8_t *)b) ? 1 : -1;
}

int median(std::vector<int> els) {
  if (els.size() == 0) return 0;
  if (els.size() == 1) return els.at(0);
  std::sort(els.begin(), els.end());
  int median;
  int size = els.size();
  if (size % 2 == 0) {
    median = els.at(size / 2);
  } else {
    median = (els.at(size / 2) + els.at((size + 2) / 2)) / 2;
  }
  return median;
}

void process_datasets(GDALDatasetH out, GDALDatasetH *datasets, int num) {
  int bands = GDALGetRasterCount(datasets[0]);
  int width = GDALGetRasterXSize(datasets[0]);
  int height = GDALGetRasterYSize(datasets[0]);
  uint16_t *outscan = (uint16_t *)calloc(width, sizeof(uint16_t));
  uint16_t *scanline[num];
  for (int i = 0; i < num; i++)
    scanline[i] = (uint16_t *)calloc(width, sizeof(uint16_t));

  // for each band
  for (int b = 1; b <= bands; b++) {
    // and each row
    for (int y = 0; y < height; y++) {
      // grab the scanlines
      for (int d = 0; d < num; d++) {
        GDALRasterBandH band = GDALGetRasterBand(datasets[d], b);
        GDALRasterIO(band, GF_Read, 0, y, width, 1, scanline[d], width, 1,
                     GDT_UInt16, 0, 0);
      }

      // populate the out scanline with the average value across datasets
      for (int x = 0; x < width; x++) {
        uint16_t pixels[num];
        for (int d = 0; d < num; d++) pixels[d] = 0;
        int size = 0;
        for (int d = 0; d < num; d++) {
          if (scanline[d][x] > 0) {
            pixels[size] = scanline[d][x];
            size++;
          }
        }

        // average the pixels
        std::vector<int> sorted;
        for (int i = 0; i < size; i++) sorted.push_back(pixels[i]);
        int med = median(sorted);

        std::vector<int> deviations;
        for (int i = 0; i < size; i++)
          deviations.push_back(std::abs(sorted.at(i) - med));
        int mad = median(deviations);

        // Boris Iglewicz and David Hoaglin (1993), "Volume 16: How to Detect
        // and Handle Outliers"
        int tot = 0;
        int real_size = size;
        for (int i = 0; i < size; i++) {
          float m = mad != 0
                        ? std::abs(0.6745 * ((float)pixels[i] - (float)med) /
                                   (float)mad)
                        : 0;
          if (m != 0 && m < 3.5) {
            tot += pixels[i];
          } else {
            real_size--;
          }
        }
        outscan[x] = real_size != 0 ? tot / real_size : pixels[0];
      }
      GDALRasterBandH band = GDALGetRasterBand(out, b);
      GDALRasterIO(band, GF_Write, 0, y, width, 1, outscan, width, 1,
                   GDT_UInt16, 0, 0);
      printf("Band %i: %.2f%%\r", b, (float)y / (float)height * 100);
      fflush(stdout);
    }
    printf("Band %i: done   \n", b);
  }

  for (int i = 0; i < num; i++) free(scanline[i]);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("usage: schooner-blend <blend-datasets*> out.tif");
    return -1;
  };
  GDALAllRegister();
  int num = argc - 2;
  char *out;
  char **options = NULL;
  GDALDatasetH datasets[num], outds = NULL;
  memset(datasets, 0, sizeof(datasets));

  bool err = open_datasets(datasets, num, argv + 1);
  check(err, "Could not open all datasets.\n");

  out = argv[argc - 1];

  options = CSLSetNameValue(options, "PHOTOMETRIC", "RGB");

  outds = GDALCreate(GDALGetDriverByName("GTiff"), out,
                     GDALGetRasterXSize(datasets[0]),
                     GDALGetRasterYSize(datasets[0]),
                     GDALGetRasterCount(datasets[0]), GDT_UInt16, options);
  check(outds, "Couldn't create the output dataset %s.\n", out);

  process_datasets(outds, datasets, num);
  assign_projection(datasets[0], outds);
  close_datasets(datasets, num);
  return 0;
error:
  close_datasets(datasets, num);
  if (outds) GDALClose(outds);
  return -1;
}
