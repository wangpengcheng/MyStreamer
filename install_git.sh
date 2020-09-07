#!/bin/bash

yum install curl-devel expat-devel gettext-devel openssl-devel zlib-devel asciidoc
yum install  gcc perl-ExtUtils-MakeMaker

cd /usr/local/src/
wget https://mirrors.edge.kernel.org/pub/software/scm/git/git-2.23.0.tar.xz
tar -xvf git-2.23.0.tar.xz
cd git-2.23.0/
make prefix=/usr/local/git all
make prefix=/usr/local/git install
echo "export PATH=$PATH:/usr/local/git/bin" >> /etc/profile
source /etc/profile