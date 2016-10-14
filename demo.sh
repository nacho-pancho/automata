#!/bin/bash
if [[ -f img/boat.pgm  ]]
then
  cd .
else
  cd img
  ./get_classic.sh
fi
