### `cache/`
Temporary folder used for installed `ekx` package build files, downloads, "build on demand" binaries.

### `ci/`
Development scripts required for maintaining the `ekx` only. Check build, run tests, update external source code. Folder is excluded from final package.

### `external/`
External libraries source code.

### `modules/`
TypeScript source-code for `ekx` users. TS modules and CLI entry-point.

### `packages/`
All core ekx-packages.

### `plugins/`
Plugable extension ekx-packages.

### `tools/`
`ekx` utilities source-code. Utilities will be built on demand by user host.

### `examples/`
Example projects. Excluded from `ekx` distribution.

### `docs/`
All documentation files. Excluded from distribution.

### `.changeset`
Changeset files using for `ekx` release flow. Excluded from distribution.
