# Kaidan - Cross plattform xmpp client

Installation
------------

On Debian do the following:

Create a working directory

 * mkdir src
 * cd src

Fetch swift source

 * wget https://github.com/swift/swift/archive/swift-3.0.zip
 * unzip swift-3.0.zip
 * cd swift-swift-3.0/

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


