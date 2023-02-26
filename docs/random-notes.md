# Random Notes

## Android Emulator

Create x86 image
```
$ANDROID_HOME/emulator/emulator @Nexus_S_API_29 -gpu host -no-boot-anim
```

## CMake

### C++ build speed optimization

Install CCache:
```
brew install --HEAD ccache
```

CLion: CMake -> Settings, add following to profile:
- `-DCMAKE_CXX_COMPILER_LAUNCHER=ccache`
- `-G Ninja`

## Words

`UNORM` is a float in the range of `[0, 1]`.
`SNORM` is the same but in the range of `[-1, 1]`

## Resources

- [The Book of Shaders](https://thebookofshaders.com/)

