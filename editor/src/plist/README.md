## DESCRIPTION

This is a C++ Property List (plist) serialization library (MIT license).
It supports both XML and binary versions of the plist format and is designed to
be cross platform with minimal dependencies.  The interface is simply a set of
static methods which serialize a subset of stdlib containers to plists (as well
as the inverse operation of deserializing plists into stdlib containers).

It uses boost::any as a generic type in order to support heterogenous
containers, e.g. a dictionary is a map<string, any> and an array is a
vector<any>.  The supported stdlib containers and their associations to plist
types is shown below. 

plist           C++
__________________________________________________________________________________

string          std::string 
integer         short, int, long, int32_t, int64_t (always deserializes as int64_t)
real            double, float (always deserializes as double)
dictionary      std::map<std::string, boost::any>
array           std::vector<boost::any>
date            PlistDate (included class in PlistDate.hpp)
data            std::vector<char>
boolean         bool

-----------------
USAGE
-----------------

See src/plistTests.cpp for examples of reading and writing all types to both
XML and binary.  E.g. to read a plist from disk whose root node is a
dictionary:

		map<string, boost::any> dict; 
		Plist::readPlist("binaryExample1.plist", dict);

The plist format (binary or XML) is automatically detected so call the same
readPlist method for XML

		Plist::readPlist("XMLExample1.plist", dict);

To write a plist, e.g. dictionary

		map<string, boost::any> dict;
		populateDictionary(dict);
		Plist::writePlistXML("xmlExampleWritten.plist", dict);

and for a binary plist

		Plist::writePlistBinary("binaryExampleWritten.plist", dict);

The other public methods allow for reading and writing from streams and byte
arrays.  Again, see the test suite code src/plistTests.cpp for comprehensive
examples. 

-----------------
LIMITATIONS
-----------------

Unicode strings are currently not supported. 

-----------------
INSTALL
-----------------

Simply copy src/Plist.hpp, src/PlistDate.hpp, src/pugixml.hpp,
src/pugiconfig.hpp, src/base64.hpp and src/pugixml.cpp to your project.  If you
do not have boost::any installed on your system, also grab the include/boost
folder which contains the minimum boost headers needed for boost::any.

To compile and run the test suites (test suites will also run automatically as
part of the post build process).  Note, the UnitTest++ library is required and
included.

OSX, Linux (for Linux, change OSX to Linux below):

mkdir -p OSX/Release
cd OSX/Release
cmake -DCMAKE_BUILD_TYPE=Release ../..
make
cd ../../
sh runTests.sh Release OSX 

and for Debug

mkdir -p OSX/Debug
cd OSX/Debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
make
cd ../../
sh runTests.sh Debug OSX

Windows:

cd Windows
cmake ..
start Plist.sln  (build solution)
cd ..
runTests.bat Release