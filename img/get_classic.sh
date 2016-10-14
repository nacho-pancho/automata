#!/bin/bash
file="classic_images_grayscale.zip"
wget -c http://iie.fing.edu.uy/~nacho/cosmos/download/${file}
unzip ${file}
file="classic_color.zip"
wget -c http://iie.fing.edu.uy/~nacho/cosmos/download/${file}
unzip ${file}
