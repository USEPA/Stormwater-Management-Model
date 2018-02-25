mkdir build
cd build
cmake .. -G "Visual Studio 12" -DCMAKE_INSTALL_PREFIX:PATH=./..
:: Optional If you use MSBUILD
MSBUILD.exe INSTALL.vcxproj /p:Configuration=Release
Pause
