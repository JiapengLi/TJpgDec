#!/bin/bash


JPEG_DECODER=./jpeg_decode
IMAGES=samples/*.jpg

for img in $IMAGES; do
    $JPEG_DECODER "$img" > samples/$(basename "$img" .jpg).txt
done

JPEG_DECODER=./jpeg_decode_debug
IMAGES=samples/*.jpg

for img in $IMAGES; do
    $JPEG_DECODER "$img" > samples/$(basename "$img" .jpg)-debug.txt
done
