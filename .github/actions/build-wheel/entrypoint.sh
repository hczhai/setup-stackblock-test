#!/bin/bash

set -e -x

cd /github/workspace

PYTHON_VERSION=$1
PARALLEL=$2

if [ "${PYTHON_VERSION}" = "3.6" ]; then
    PY_VER=cp36-cp36m
elif [ "${PYTHON_VERSION}" = "3.7" ]; then
    PY_VER=cp37-cp37m
elif [ "${PYTHON_VERSION}" = "3.8" ]; then
    PY_VER=cp38-cp38
elif [ "${PYTHON_VERSION}" = "3.9" ]; then
    PY_VER=cp39-cp39
elif [ "${PYTHON_VERSION}" = "3.10" ]; then
    PY_VER=cp310-cp310
fi

PY_EXE=/opt/python/"${PY_VER}"/bin/python3
sed -i "/DPYTHON_EXECUTABLE/a \                '-DPYTHON_EXECUTABLE=${PY_EXE}'," setup.py

/opt/python/"${PY_VER}"/bin/pip install --upgrade --no-cache-dir pip
/opt/python/"${PY_VER}"/bin/pip install --no-cache-dir mkl==2019 mkl-include intel-openmp cmake==3.17

yum install -y wget
PREFIX=$PWD
wget https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz
tar zxf boost_1_76_0.tar.gz
cd boost_1_76_0
bash bootstrap.sh
cd ..

if [ "${PARALLEL}" = "mpi" ]; then
    cd boost_1_76_0
    echo 'using mpi ;' >> project-config.jam
    cd ..
    yum install -y openssh-clients openssh-server
    wget https://download.open-mpi.org/release/open-mpi/v4.0/openmpi-4.0.6.tar.gz
    tar zxf openmpi-4.0.6.tar.gz
    cd openmpi-4.0.6
    ./configure --prefix=/usr/local |& tee config.out
    make -j 4 |& tee make.out
    make install |& tee install.out
    cd ..
    export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
    /opt/python/"${PY_VER}"/bin/pip install --no-cache-dir mpi4py
    sed -i '/DUSE_MKL/a \                "-DMPI=ON",' setup.py
    sed -i "s/name='stackblock'/name='stackblock-mpi'/g" setup.py
    sed -i '/for soname, src_path/a \                if any(x in soname for x in ["libmpi", "libopen-pal", "libopen-rte"]): continue' \
        $($(cat $(which auditwheel) | head -1 | awk -F'!' '{print $2}') -c "from auditwheel import repair;print(repair.__file__)")
    sed -i '/for soname, src_path/a \                if "libmpi.so" in soname: patcher.replace_needed(fn, soname, "libmpi.so")' \
        $($(cat $(which auditwheel) | head -1 | awk -F'!' '{print $2}') -c "from auditwheel import repair;print(repair.__file__)")
fi

cd boost_1_76_0
./b2 install --prefix=$PREFIX
export BOOSTROOT=~/program/boost-1.76
cd ..

sed -i '/new_soname = src_name/a \    if any(x in src_name for x in ["libmkl_avx2", "libmkl_avx512"]): new_soname = src_name' \
    $($(cat $(which auditwheel) | head -1 | awk -F'!' '{print $2}') -c "from auditwheel import repair;print(repair.__file__)")
${PY_EXE} -c 'import site; x = site.getsitepackages(); x += [xx.replace("site-packages", "dist-packages") for xx in x]; print("*".join(x))' > /tmp/ptmp
sed -i '/rpath_set\[rpath\]/a \    import site\n    for x in set(["../lib" + p.split("lib")[-1] for p in open("/tmp/ptmp").read().strip().split("*")]): rpath_set[rpath.replace("../..", x)] = ""' \
    $($(cat $(which auditwheel) | head -1 | awk -F'!' '{print $2}') -c "from auditwheel import repair;print(repair.__file__)")

/opt/python/"${PY_VER}"/bin/pip wheel . -w ./dist --no-deps

find . -type f -iname "*-linux*.whl" -exec sh -c "auditwheel repair '{}' -w \$(dirname '{}') --plat '${PLAT}'" \;
find . -type f -iname "*-linux*.whl" -exec rm {} \;
find . -type f -iname "*-manylinux*.whl"

rm /tmp/ptmp
