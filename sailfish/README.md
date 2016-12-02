# Kaidan Sailfish adaptions

## Installation

On Linux do the following:

Get and install Sailfish OS mersdk (tested with version 1608)

Ssh into mersdk and do the following in a newly created directory:

Fetch swift source

 * `wget https://swift.im/downloads/releases/swift-3.0/swift-3.0.tar.gz`
 * `tar -xzvf swift-3.0.tar.gz`
 * `cd swift-3.0/`

Install all dependencies to build swiften

 * `sb2 -t SailfishOS-armv7hl -m sdk-install -R zypper in openssl-devel`

Patch SConstruct file to do a PIC build of the library archive

Add
 * `env.Append(CCFLAGS='-fPIC')`
under the line 'env.SConscript = SConscript' on line 14

Build Swiften Library

 * `sb2 -t SailfishOS-armv7hl /bin/bash ./scons Swiften`

Get Kaidan source code

 * `cd ..`
 * `git clone https://github.com/KaidanIM/Kaidan`
 * `cd Kaidan`
 * `mb2 -t SailfishOS-armv7hl build`

