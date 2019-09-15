# Maintainer: Alex <alex1701c.dev[at]gmx.net>
pkgname=krunner-emojirunner
pkgver=1.1.3
pkgrel=1
pkgdesc="Select emojis and copy/paste them"
arch=("any") 
url=https://github.com/alex1701c/EmojiRunner
license=(GLP3)
groups=()
depends=(krunner xdotool)
makedepends=(cmake extra-cmake-modules ki18n qt5-base qt5-tools)
optdepends=()
provides=()
conflicts=()
replaces=()
backup=()
options=()
install=
changelog=
source=("https://github.com/alex1701c/EmojiRunner/archive/1.1.3.tar.gz")
noextract=()
md5sums=(SKIP)

prepare(){
	cd  "EmojiRunner-$pkgver"
	mkdir -p build
}

build() {
	cd "EmojiRunner-$pkgver/build"
	cmake -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins` -DCMAKE_BUILD_TYPE=Release ..
	make -j $(nproc)
}

package() {
	cd  "$srcdir/EmojiRunner-$pkgver/build"
	sudo make install
}
