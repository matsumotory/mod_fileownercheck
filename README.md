# mod_fileownercheck

uid check for opened r->filename at output filter phase.

## How to Use
### Quick Install
```
apxs -c -i mod_fileownercheck.c
```
### Config
```
SetOutputFilter FILEOWNERCHECK
```
