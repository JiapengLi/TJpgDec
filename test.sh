#!/bin/bash


JPEG_DECODER=./jpeg_decode
IMAGES=samples/*.jpg

for img in $IMAGES; do
    $JPEG_DECODER "$img"
done
