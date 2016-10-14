#!/bin/bash
wget -c http://sipi.usc.edu/database/misc.tar.gz
tar xzf misc.tar.gz
cd misc
for i in *.tiff 
do
  convert ${i} ${i/tiff/ppm}
done

