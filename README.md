# ekx


```bash
pip3 install --upgrade conan
conan remote add -f eliasku https://api.bintray.com/conan/eliasku/public-conan
conan install -if cmake-build-debug -pr clion-debug -b missing .
```



add tooling root to environment
```
open -e ~/.bash_profile
```

add lines:
```
source ~/highduck/ekx/editor/env.sh
```