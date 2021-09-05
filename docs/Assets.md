# Assets

## Building

- `export/android/assets/*`

### Embedded Pack

#### Android embedded assets

- assets1/ (add that as `assets` source root)
  - pack1/
  - pack2/
  - root-files...

- assets2/ (add that as `assets` source root)
    - pack3/
    - pack4/

After Android build we got the following structure, and all packs are available in AssetManager

- bundle-dir/
  - pack1/
  - pack2/
  - root-files...
  - pack3/
  - pack4/