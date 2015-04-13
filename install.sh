#! /bin/sh

mkdir -pv $1/app
mkdir -pv $1/mod
mkdir -pv $1/data
mkdir -pv $1/data/bees
mkdir -pv $1/data/bees/scalers
mkdir -pv $1/data/bees/scenes

cp -v utils/param_scaling/scaler_*.dat $1/data/bees/scalers/
cp -v apps/bees/aleph-bees*.hex $1/app/
cp -v modules/waves/aleph-waves.ldr $1/mod/
cp -v modules/waves/aleph-waves.dsc $1/mod/
cp -v modules/lines/aleph-lines.ldr $1/mod/
cp -v modules/lines/aleph-lines.dsc $1/mod/
cp -v modules/dsyn/aleph-dsyn.ldr $1/mod/
cp -v modules/dsyn/aleph-dsyn.dsc $1/mod/
