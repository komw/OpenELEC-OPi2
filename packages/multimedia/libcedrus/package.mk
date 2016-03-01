################################################################################
#      This file is part of OpenELEC - http://www.openelec.tv
#      Copyright (C) 2009-2014 Stephan Raue (stephan@openelec.tv)
#
#  OpenELEC is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 2 of the License, or
#  (at your option) any later version.
#
#  OpenELEC is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with OpenELEC.  If not, see <http://www.gnu.org/licenses/>.
################################################################################

PKG_NAME="libcedrus"
PKG_VERSION="e129e79337fc9073ddaf56fc680f2338dac8960b"
PKG_REV="1"
PKG_ARCH="any"
PKG_LICENSE="GPL"
PKG_SITE="https://github.com/linux-sunxi/libcedrus"
PKG_URL="https://github.com/linux-sunxi/libcedrus/archive/$PKG_VERSION.tar.gz"
PKG_DEPENDS_TARGET="toolchain"
PKG_PRIORITY="optional"
PKG_SECTION="multimedia"
PKG_SHORTDESC="libcedrus: sunxi Video Engine library"
PKG_LONGDESC="libcedrus: sunxi Video Engine library"

PKG_IS_ADDON="no"
PKG_AUTORECONF="no"

unpack() {
  tar xf $ROOT/$SOURCES/$PKG_NAME/$PKG_VERSION.tar.gz -C $ROOT/$BUILD
}
	