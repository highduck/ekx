# C++ build speed optimization

Install CCache:
```
brew install --HEAD ccache
```

CLion: CMake -> Settings, add following to profile:
- `-DCMAKE_CXX_COMPILER_LAUNCHER=ccache`
- `-G "Ninja"`