Schooner-tk
===========

A collection of utilities for averaging, color correcting, and removing
artifacts from satellite images. It complements [GDAL](http://www.gdal.org/) and
[landsat util](https://github.com/developmentseed/landsat-util).

Installation
------------

Schooner-tk requires OpenCV 3. To install OpenCV 3 on **OSX**, use Homebrew:

    brew install opencv3

unless you want to fight with `CFLAGS`, `LDLIBS`, and `PKG_CONFIG_PATH` go ahead and link opencv3

    brew link opencv3 --force

Be forewarned this will mess with anything that relies on opencv 2.

For **linux distributions**, use `apt-get`, `pacman`, `yum`, or whichever package manager is preferred.

Once OpenCV is installed, **clone the repository**.

    git clone git@github.com:propublica/schooner-tk.git
    cd schooner-tk

Use **make** to compile and install schooner.

    make && make install

Once installed, each command can be called from your terminal with:

    schooner-<utilityname>

Utilities
---------

#### Schooner-blend

`schooner-blend` averages multiple datasets together on a per pixel basis
in order to remove temporary artifacts such as small clouds, airplane
contrails, and sensor malfunctions.

Each input dataset must be the same size and in the same projection.

#### Schooner-cloud

`schooner-cloud` creates a cloud and snow mask from a Landsat 8 Quality
Assessment band.

#### Schooner-contrast

`schooner-contrast` attempts to automatically color correct a landsat image.
It uses two algorithms, the first is CLAHE (Contrast Limited Adaptive Histogram
Equalization) which attempts to color correct an image while improving the
local contrast of the image.

The other is simple histogram stretching and clipping. The tool calculates a
histogram by band, and stretches the raster values to the range 0 to 255. It
also discards the top and bottom 0.05% of values.

#### Schooner-multibalance

`schooner-multibalance` automatically corrects each band of many datasets so
that each dataset has a similar color profile. This is useful as a preprocessing
step to schooner-blend.

#### Schooner-stich

`schooner-stitch` seamlessly stitches multiple images together.

Learn more
----------

For more detail, please see the [official documentation](https://propublica.github.io/schooner-tk/).
For an example of a schooner-powered workflow, see [jqtr.de/schooner/](http://jqtr.de/technical/2015/05/27/schooner.html).
