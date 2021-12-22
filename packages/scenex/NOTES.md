# C++ build speed optimization

Install CCache:
```
brew install --HEAD ccache
```

CLion: CMake -> Settings, add following to profile:
- `-DCMAKE_CXX_COMPILER_LAUNCHER=ccache`
- `-G Ninja`



# Plugins

References:
- https://github.com/oxygine
- ludei/atomic-plugins: https://github.com/ludei/atomic-plugins-ads

# Android Emulator

Create x86 image
```
$ANDROID_HOME/emulator/emulator @Nexus_S_API_29 -gpu host -no-boot-anim
```


