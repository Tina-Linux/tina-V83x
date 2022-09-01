> [V831/V833 的 SDK 的 kernel & package 的开发方法](https://www.cnblogs.com/juwan/p/15226245.html)

# tina-V833

![GitHub repo size in bytes](https://img.shields.io/github/repo-size/Tina-V833/tina-V833?style=for-the-badge)
![GitHub language count](https://img.shields.io/github/languages/count/Tina-V833/tina-V833?style=for-the-badge)

This repo aim to integrate the codes for V833 Tina from https://github.com/orgs/lindenis-org/ and try to build a full image.

# Usage

## Get the Code
```shell
git clone --recursive https://github.com/Tina-Linux/tina-V83x
cd tina-V83x
```

After clone you'll have a 4.7GB source code directory, after download all packages you'll have a 16GB directory. Make sure you have enough space for the source code.

## Install requirement

Recommend Host OS: Ubuntu 16.04

```shell
sudo apt-get install build-essential subversion git-core libncurses5-dev zlib1g-dev gawk flex quilt libssl-dev xsltproc libxml-parser-perl mercurial bzr ecj cvs unzip lib32z1 lib32z1-dev lib32stdc++6 libstdc++6 -y
```

## Download dl/ files

The dl folder is very large, with 4.7GiB, so it is uploaded to the network disk. Please download and unzip it from the network disk and put it in the dl folder

https://drive.google.com/drive/folders/1PzVQIhMdAj6FnqjhwyWq9vbwgjbGUWSj?usp=sharing

## Build and Pack
```shell
source build/envsetup.sh
lunch
make -j32
pack
```
