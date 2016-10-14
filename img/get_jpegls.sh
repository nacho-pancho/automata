#!/bin/bash
file="jpegls_images_grayscale.zip"
wget -c http://iie.fing.edu.uy/~nacho/cosmos/download/${file}
unzip ${file}
file="jpegls_images_cmyk.zip"
wget -c http://iie.fing.edu.uy/~nacho/cosmos/download/${file}
unzip ${file}

