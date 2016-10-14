#!/bin/bash
# Taken from http://sun.aei.polsl.pl/~rstaros/mednat/index.html
# see that URL for proper citation

mkdir mednat
cd mednat
wget -c http://sun.aei.polsl.pl/~rstaros/mednat/mednat_set_decription.pdf
wget -c http://sun.aei.polsl.pl/~rstaros/mednat/mednat-natural.zip
wget -c http://sun.aei.polsl.pl/~rstaros/mednat/mednat-medical.zip
wget -c http://sun.aei.polsl.pl/~rstaros/mednat/mednat-other.zip
for i in *.zip; do unzip $i; done
