# Kaidan - Cross platform XMPP client

Installation
------------

On Debian do the following:

Create a working directory

 * mkdir src
 * cd src

Fetch swift source

 * wget https://swift.im/downloads/releases/swift-3.0/swift-3.0.tar.gz
 * tar -xzvf swift-3.0.tar.gz
 * cd swift-3.0/

Install all dependencies to build swiften

 * ./BuildTools/InstallSwiftDependencies.sh
 * ./scons Swiften

Get Kaidan source code

 * cd ..
 * git clone https://github.com/KaidanIM/Kaidan

Either

 * open pro file within qtreator

or use command line

 * cd Kaidan/app
 * qmake
 * make


