#include <cpl_error.h>
#include <cpl_conv.h>
#include <gdal.h>
#define check(err, mess, args...) if(!(err)) { fprintf(stderr, mess, ##args); goto error; }

int
main(int argc, char *argv[]) {
  GDALDatasetH mask = NULL;
  GDALDatasetH out = NULL;
  uint16_t *scanline = NULL;
  uint8_t *tiny_scanline = NULL;
  int width, height;
  check(argc == 3,
    "usage: quality-layer.tif mask.tif\n"
    "run this after ok?\n"
    "   gdal_calc.py -A mask.tif -B ~/landsat.tif --calc=\"A*B\" --allBands=B --overwrite --type=Byte"
    "note: this is in the landsat projection not 3857, so reproject it too"
  );

  GDALAllRegister();
  mask = GDALOpen(argv[1], GA_ReadOnly);
  width = GDALGetRasterXSize(mask), height = GDALGetRasterYSize(mask);
  check(mask, "couldn't open %s", argv[1]);
  out = GDALCreate(GDALGetDriverByName("GTiff"), argv[2], width, height, 1, GDT_UInt16, NULL);
  check(out, "couldn't create %s", argv[2]);
  scanline = (uint16_t *)calloc(GDALGetRasterXSize(mask), sizeof(uint16_t));
  tiny_scanline = (uint8_t *)calloc(GDALGetRasterXSize(mask), sizeof(uint16_t));

  for(int y = 0; y < height; y++) {
    printf("row: %i\r", y + 1);
    fflush(stdout);
    GDALRasterIO(GDALGetRasterBand(mask, 1), GF_Read, 0, y, width, 1, scanline, width, 1, GDT_UInt16, 0, 0);
    for(int x = 0; x < width; x++){
      if((scanline[x] & (1 << 15)) || (scanline[x] & (1 << 13)) || scanline[x] & 1){
        tiny_scanline[x] = 0;
      } else {
        tiny_scanline[x] = 1;
      }
    }
    GDALRasterIO(GDALGetRasterBand(out, 1), GF_Write, 0, y, width, 1, tiny_scanline, width, 1, GDT_Byte, 0, 0);
  }
  puts("");

  double transform[6];
  if(GDALGetGeoTransform(mask, transform) == CE_None)
    GDALSetGeoTransform(out, transform);

  if(GDALGetProjectionRef(mask) != NULL)
    GDALSetProjection(out, GDALGetProjectionRef(mask));


  GDALClose(out);
  GDALClose(mask);
  free(scanline);
  free(tiny_scanline);
  return -1;
error:
  if(mask) GDALClose(mask);
  return -1;
}