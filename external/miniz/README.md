# miniz

Version: 2.1.0

Patch `miniz_zip.c`:
```c
cdir_ofs = MZ_READ_LE32(pBuf + MZ_ZIP_ECDH_CDIR_OFS_OFS);
// TODO: PATCH!
if(cdir_ofs + cdir_size > pZip->m_archive_size) {
    cdir_size = pZip->m_archive_size - cdir_ofs;
}
