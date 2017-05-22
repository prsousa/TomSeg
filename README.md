# TomSeg

TomSeg is an efficient cross-platorm software tool to segment (electron) tomograms, allowing to:

  - import RAW tomogram data (i.e., stacks of tomogram slices);
  - align the data using reference markers, or global signals;
  - view and navigate across the slices;
  - test a new 3D segmentation procedure, based on region growing techniques;
  - export the results as a stack of individual images, or as a 3D volume in MRC files.


## Characteristics
  - This software includes a new efficient algorithmic approach to segment 3D volumes;
  - It supports multi-threaded parallelism, using OpenMP;
  - OpenCV is used to perform basic operations such as image loading and represent the images as matrices;
  - Both GUI and CLI are available:
    - The GUI depends on QT,
    - The CLI depends on boost\_program\_options;
  - This software is capable of persist and resume the work at any time;
  - Was developed to allow easy integration of new features and alternatives;
  - Supports CUDA;
  - It is open-source!

## Installation
### Command Line Interface
With OpenCV and boost\_program\_options installed:

```sh
$ git clone https://github.com/prsousa/TomSeg.git
$ cd TomSeg
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./TomSeg --help
```

### Graphical User Interface
Clone and open the project with Qt Creator, or:

```sh
$ git clone https://github.com/prsousa/TomSeg.git
$ cd TomSeg
$ mkdir build
$ cd build
$ qmake ..
$ make
$ ./TomSeg
```


License
----

GNU General Public License v3.0