#!/bin/bash

BINUTILS_VERSION="2.19.1"
GMP_VERSION="4.3.1"
MPFR_VERSION="2.4.1"
GCC_VERSION="4.4.0"
NEWLIB_VERSION="1.17.0"
GDB_VERSION="6.8"
PREFIX="/usr/local/crossgcc-powerpc-eabi-$(date +%Y-%m-%d)"
TARGET="powerpc-eabi"
MAKE_FLAGS="-j 4"
CFLAGS="-pipe"

if !(test -d downloads) then
	mkdir downloads
fi
cd downloads
echo
echo "Checking source packages..."
echo "---------------------------"
echo

if !(test -f binutils-$BINUTILS_VERSION.tar.bz2) then
	wget ftp://sourceware.org/pub/binutils/releases/binutils-$BINUTILS_VERSION.tar.bz2
else
	echo "binutils-$BINUTILS_VERSION.tar.bz2 already available."
fi

if !(test -f gmp-${GMP_VERSION}.tar.bz2) then
	wget ftp://ftp-stud.fht-esslingen.de/pub/Mirrors/ftp.gnu.org/gmp/gmp-${GMP_VERSION}.tar.bz2
else
	echo "gmp-${GMP_VERSION}.tar.bz2 already available."
fi

if !(test -f mpfr-${MPFR_VERSION}.tar.bz2) then
	wget http://www.mpfr.org/mpfr-current/mpfr-${MPFR_VERSION}.tar.bz2
else
	echo "mpfr-${MPFR_VERSION}.tar.bz2 already available."
fi

if !(test -f gcc-core-$GCC_VERSION.tar.bz2) then
	wget ftp://ftp.fu-berlin.de/unix/languages/gcc/releases/gcc-$GCC_VERSION/gcc-core-$GCC_VERSION.tar.bz2
else
	echo "gcc-core-$GCC_VERSION.tar.bz2 already available"
fi

if !(test -f gcc-g++-$GCC_VERSION.tar.bz2) then
	wget ftp://ftp.fu-berlin.de/unix/languages/gcc/releases/gcc-$GCC_VERSION/gcc-g++-$GCC_VERSION.tar.bz2
else
	echo "gcc-g++-$GCC_VERSION.tar.bz2 already-available"
fi

if !(test -f newlib-$NEWLIB_VERSION.tar.gz) then
	wget ftp://sources.redhat.com/pub/newlib/newlib-$NEWLIB_VERSION.tar.gz
else
	echo "newlib-$NEWLIB_VERSION.tar.gz already-available"
fi

if !(test -f gdb-$GDB_VERSION.tar.bz2) then
	wget ftp://ftp-stud.fht-esslingen.de/pub/Mirrors/ftp.gnu.org/gdb/gdb-$GDB_VERSION.tar.bz2
else
	echo "gdb-$GDB_VERSION.tar.bz2 already-available"
fi

cd ..

echo
echo "Checking source tree"
echo "--------------------"
echo

if !(test -d sources) then
	mkdir sources
fi

cd sources

if !(test -d binutils-$BINUTILS_VERSION) then
	echo "Extracting binutils-$BINUTILS_VERSION"
	tar -xjf ../downloads/binutils-$BINUTILS_VERSION.tar.bz2
else
	echo "binutils-$BINUTILS_VERSION source tree already available"
fi

if !(test -d newlib-$NEWLIB_VERSION) then
	echo "Extracting newlib-$NEWLIB_VERSION"
	tar -xzf ../downloads/newlib-$NEWLIB_VERSION.tar.gz
else
	echo "newlib-$NEWLIB_VERSION source tree already available"
fi

if !(test -d gmp-$GMP_VERSION) then
	echo "Extracting gmp-$GMP_VERSION"
	tar -xjf ../downloads/gmp-$GMP_VERSION.tar.bz2
else
	echo "gmp-$GMP_VERSION source tree already available"
fi

if !(test -d mpfr-$MPFR_VERSION) then
	echo "Extracting mpfr-$MPFR_VERSION"
	tar -xjf ../downloads/mpfr-$MPFR_VERSION.tar.bz2
else
	echo "mpfr-$MPFR_VERSION source tree already available"
fi

if !(test -d gcc-$GCC_VERSION) then
	echo "Extracting gcc-core-$GCC_VERSION"
	tar -xjf ../downloads/gcc-core-$GCC_VERSION.tar.bz2
	echo "Extracting gcc-g++-$GCC_VERSION"
	tar -xjf ../downloads/gcc-g++-$GCC_VERSION.tar.bz2
	echo "Linking newlib into gcc source tree..."
	cd gcc-$GCC_VERSION
	ln -s ../newlib-$NEWLIB_VERSION/* . &> /dev/null
	ln -s ../gmp-$GMP_VERSION gmp
	ln -s ../mpfr-$MPFR_VERSION mpfr
	cd ..
else
	echo "gcc-$GCC_VERSION source tree already available"
fi

if !(test -d gdb-$GDB_VERSION) then
	echo "Extracting gdb-$GDB_VERSION.tar.bz2"
	tar -xjf ../downloads/gdb-$GDB_VERSION.tar.bz2
else
	echo "gdb-$GDB_VERSION source tree already available"
fi

cd ..

echo
echo "Checking Build Tree"
echo "-------------------"
echo

if !(test -d build) then
	mkdir build
fi

cd build

if !(test -d binutils-$BINUTILS_VERSION) then
	mkdir binutils-$BINUTILS_VERSION
	cd binutils-$BINUTILS_VERSION
	echo "Configuring binutils-$BINUTILS_VERSION..."
	../../sources/binutils-$BINUTILS_VERSION/configure --prefix=$PREFIX --target=$TARGET --disable-nls --disable-shared &> conf.log
	echo "Building binutils..."
	make ${MAKE_FLAGS} CFLAGS="$CFLAGS" &> build.log
	if [ "${?}" -ne "0" ]; then
		echo "failed"
		echo "Check log for more information"
		exit
	fi
	echo "Installing binutils-$BINUTILS_VERSION..."
	sudo make ${MAKE_FLAGS} install &> install.log
	cd ..
else
	cd binutils-$BINUTILS_VERSION
	echo "binutils-$BINUTILS_VERSION already built"
	echo "Installing binutils-$BINUTILS_VERSION..."
	sudo make install &> install.log
	if [ "${?}" -ne "0" ]; then
		echo "failed"
		echo "Check log for more information"
		exit
	fi
	cd ..
fi

if !(test -d gcc-$GCC_VERSION) then
	mkdir gcc-$GCC_VERSION
	cd gcc-$GCC_VERSION
	echo "Configuring gcc-$GCC_VERSION..."
	../../sources/gcc-$GCC_VERSION/configure --prefix=$PREFIX --target=$TARGET --disable-nls --disable-shared --with-gnu-ar --with-gnu-as --with-gnu-ld --with-newlib &> conf.log
	echo "Building gcc..."
	make ${MAKE_FLAGS} CFLAGS="$CFLAGS" &> build.log
	if [ "${?}" -ne "0" ]; then
		echo "failed"
		echo "Check log for more information"
		exit
	fi
	echo "Installing gcc-$GCC_VERSION..."
	sudo make ${MAKE_FLAGS} install &> install.log
	if [ "${?}" -ne "0" ]; then
		echo "failed"
		echo "Check log for more information"
		exit
	fi
	cd ..
else
	cd gcc-$GCC_VERSION
	echo "gcc-$GCC_VERSION already built"
	echo "Installing gcc-$GCC_VERSION..."
	sudo make install &> install.log
	if [ "${?}" -ne "0" ]; then
		echo "failed"
		echo "Check log for more information"
		exit
	fi
	cd ..
fi

if !(test -d gdb-$GDB_VERSION) then
	mkdir gdb-$GDB_VERSION
	cd gdb-$GDB_VERSION
	echo "Configuring gdb-$GDB_VERSION..."
	../../sources/gdb-$GDB_VERSION/configure --prefix=$PREFIX --target=$TARGET --disable-nls --disable-shared &> build.log
	echo "Building gdb-$GDB_VERSION..."
	make ${MAKE_FLAGS} CFLAGS="$CFLAGS" &> build.log
	if [ "${?}" -ne "0" ]; then
		echo "failed"
		echo "Check log for more information"
		exit
	fi
	echo "Installing gdb-$GDB_VERSION"
	sudo make install &> install.log
	if [ "${?}" -ne "0" ]; then
		echo "failed"
		echo "Check log for more information"
		exit
	fi
	cd ..
else
	cd gdb-$GDB_VERSION
	echo "gdb-$GDB_VERSION already built"
	echo "Installing gdb-$GDB_VERSION"
	sudo make install &> install.log
	if [ "${?}" -ne "0" ]; then
		echo "failed"
		echo "Check log for more information"
		exit
	fi
	cd ..
fi
