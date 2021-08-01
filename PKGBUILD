# Maintainer: fpliu <leleliu008@gmail.com>

pkgname=('zpkg')
pkgver='0.1.0'
pkgrel=1
pkgdesc="a lightweight, portable package manager for UNIX"
arch=('any')
license=('custom')
url="https://github.com/leleliu008/zpkg"
makedepends=()
source=("https://github.com/leleliu008/zpkg/releases/download/v${pkgver}/${pkgname}-${pkgver}.tar.gz")
sha256sums=('e4f49362fca1ff489b3e003e7a82c80a76702576a91482a322096f5258d8cef8')

build() {
    true
}

check() {
    cd "${srcdir}"
    bin/zpkg -V
}

package() {
    mkdir -p ${pkgdir}/usr/local/bin
    mkdir -p ${pkgdir}/usr/local/share/zsh/site-functions
    
    cp -f ${srcdir}/bin/zpkg ${pkgdir}/usr/local/bin/
    cp -f ${srcdir}/zsh-completion/_zpkg ${pkgdir}/usr/local/share/zsh/site-functions/
}
