#!/bin/bash

KAIDAN_SOURCES=$(dirname "$(readlink -f "${0}")")/..

if [ ! -d "$KAIDAN_SOURCES/misc/android/res/mipmap-xxxhdpi" ]; then
    echo "*****************************************"
    echo "Rendering logos"
    echo "*****************************************"
    rendersvg() {
        inkscape -z -e $2 -w $3 -h $3 $1
        optipng -quiet $2
    }

    androidlogo() {
        mkdir $KAIDAN_SOURCES/misc/android/res/mipmap-$1
        rendersvg $KAIDAN_SOURCES/misc/kaidan-small-margin.svg "$KAIDAN_SOURCES/misc/android/res/mipmap-$1/icon.png" $2
        rendersvg $KAIDAN_SOURCES/misc/kaidan-small-margin.svg "$KAIDAN_SOURCES/misc/android/res/mipmap-$1/logo.png" $(( $2 * 4 ))
    }

    androidlogo ldpi 36
    androidlogo mdpi 48
    androidlogo hdpi 72
    androidlogo xhdpi 96
    androidlogo xxhdpi 144
    androidlogo xxxhdpi 192
fi
