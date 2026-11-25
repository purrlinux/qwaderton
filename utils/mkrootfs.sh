#!/bin/sh
set -e

mkdir -p dist/rootfs/
export SPM_ROOT="$(realpath dist/rootfs)"

for package in \
    /var/cache/pkg/amutils#0.1-1.spm \
    /var/cache/pkg/baselayout#1-2.spm \
    /var/cache/pkg/busybox#1.38.0-4.spm \
    /var/cache/pkg/bzip2#1.0.8-1.spm \
    /var/cache/pkg/ca-certificates#20261105-1.spm \
    /var/cache/pkg/curl#8.23.0-1.spm \
    /var/cache/pkg/expat#2.11.0-1.spm \
    /var/cache/pkg/file#5.48-2.spm \
    /var/cache/pkg/libarchive#3.8.11-1.spm \
    /var/cache/pkg/libressl#4.4.1-1.spm \
    /var/cache/pkg/lz4#1.11.1-1.spm \
    /var/cache/pkg/musl#1.2.6-4.spm \
    /var/cache/pkg/musl-locales#0.1.0-1.spm \
    /var/cache/pkg/spm#1.0.1-2.spm \
    /var/cache/pkg/xz#5.8.4-1.spm \
    /var/cache/pkg/zstd#1.6.0-1.spm
do
    spm -i "$package"
done
