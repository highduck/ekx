# ekx third-party

C++ modules via NPM packages

[![Build](https://github.com/highduck/npm-deps/actions/workflows/build.yml/badge.svg)](https://github.com/highduck/npm-deps/actions/workflows/build.yml)

## Versions

- package version **is not equal** to packaged library version
- package version currently just expose **packaging recipe versioning**

## Release Flow

- regular updates should be done via `changeset` command and pushing to master, or PR to master, then Ver
- micro fixes could be made faster with manual steps `changeset version` and push changes to master for auto-check and publishing

### TODO

- cleanup task for packages
- test only packages changed since previous release, or unreleased
- rename repo
