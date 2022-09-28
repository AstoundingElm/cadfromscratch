set echo on

echo "Building assemblies..."

cFilenames="/home/petermiller/Desktop/4coder/cadfromscratch/src/main.cpp"

echo "Files:" $cFilenames
cd src
assembly="engine"
compilerFlags="-g -Wall  -fPIC "

#-Warc  -Wextra
# -fms-extensions 
# -Wall -Werror

linkerFlags="  -lX11  -lX11 -lGL -lm -DGL_GLEXT_PROTOTYPES -L/usr/X11R6/lib   " 

clang++ $cFilenames $compilerFlags -o $assembly  $linkerFlags 
cppcheck --enable=all --inconclusive -v main.cpp

#valgrind --tool=memcheck --leak-check=yes --show-reachable=yes #--num-callers=20 --track-fds=yes ./engine