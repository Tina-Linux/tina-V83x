# tina-V833

This repo aim to integrate the codes for V833 Tina from https://github.com/orgs/lindenis-org/ and try to build a full image.

# Usage

## Get the Code
```shell
git clone --recursive https://github.com/Tina-V833/tina-V833
cd tina-V833 
```

## Install requirement

Recommend Host OS: Ubuntu 16.04

```shell
sudo apt-get install build-essential subversion git-core libncurses5-dev zlib1g-dev gawk flex quilt libssl-dev xsltproc libxml-parser-perl mercurial bzr ecj cvs unzip lib32z1 lib32z1-dev lib32stdc++6 libstdc++6 -y
```

## Build and Pack
```shell
source build/envsetup.sh
lunch
make -j32
pack
```
