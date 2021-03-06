################################################################################
#      This file is part of OpenELEC - http://www.openelec.tv
#      Copyright (C) 2009-2016 Stephan Raue (stephan@openelec.tv)
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

PKG_NAME="media_build"
PKG_VERSION="66f4030"
MEDIA_BUILD_VERSION="2014-10-21-1ef2496"
PKG_REV="1"
PKG_ARCH="any"
PKG_LICENSE="GPL"
PKG_SITE="http://git.linuxtv.org/media_build.git"
PKG_URL="$DISTRO_SRC/${PKG_NAME}-${PKG_VERSION}.tar.xz"
PKG_DEPENDS_TARGET=""
PKG_BUILD_DEPENDS_TARGET="toolchain linux"
PKG_PRIORITY="optional"
PKG_SECTION="driver"
PKG_SHORTDESC="Build system to use the latest experimental drivers/patches without needing to replace the entire Kernel"
PKG_LONGDESC="Build system to use the latest experimental drivers/patches without needing to replace the entire Kernel"
PKG_IS_ADDON="no"
PKG_AUTORECONF="no"

pre_make_target() {
  export KERNEL_VER=$(get_module_dir)
  # dont use our LDFLAGS, use the KERNEL LDFLAGS
  export LDFLAGS=""
}

make_target() {
  $SED -i  -e "/^LATEST_TAR/s/-LATEST/-$MEDIA_BUILD_VERSION/g" linux/Makefile

  make VER=$KERNEL_VER SRCDIR=$(kernel_path) -C linux/ download
  make VER=$KERNEL_VER SRCDIR=$(kernel_path) -C linux/ untar

  make VER=$KERNEL_VER SRCDIR=$(kernel_path) -C v4l/ Makefile.media
  patch v4l/Makefile.media < $PROJECT_DIR/$PROJECT/packages/media_build/Makefile.media.H3.patch

  make VER=$KERNEL_VER SRCDIR=$(kernel_path) allyesconfig
  make VER=$KERNEL_VER SRCDIR=$(kernel_path)
}

makeinstall_target() {
  mkdir -p $INSTALL/lib/modules/$KERNEL_VER/updates/media_build
  find $ROOT/$PKG_BUILD/v4l/ -name \*.ko -exec strip --strip-debug {} \;
  find $ROOT/$PKG_BUILD/v4l/ -name \*.ko -exec cp {} $INSTALL/lib/modules/$KERNEL_VER/updates/media_build \;
}
