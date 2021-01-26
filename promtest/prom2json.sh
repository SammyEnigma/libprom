#!/bin/ksh93

# promtest requires the prom2json binary - so fetch it from github if n/a

SDIR=${.sh.file%/*}/..
PROM2JSON=${ whence prom2json ; }
[[ -n ${PROM2JSON} || -x ${SDIR}/bin/prom2json ]] && return 0

VERSION='1.3.0'
ARCH='amd64'
OS='linux'

BN="prom2json-${VERSION}.${OS}-${ARCH}"
ARC=https://github.com/prometheus/prom2json/releases/download/v${VERSION}/${BN}.tar.gz
wget -O ${SDIR}/prom2json.tar.gz ${ARC} || return 1
tar xzf ${SDIR}/prom2json.tar.gz ${BN}/prom2json
[[ -e ${SDIR}/bin ]] || mkdir ${SDIR}/bin
mv ${BN}/prom2json ${SDIR}/bin/
rmdir ${BN}
rm -f ${SDIR}/prom2json.tar.gz
