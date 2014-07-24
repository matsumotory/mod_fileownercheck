# mod_fileownercheck

uid check for opened r->filename at output filter phase.

## How to Use
### Quick Install
```
apxs -c -i mod_fileownercheck.c
```
### Config
#### All request
```
SetOutputFilter FILEOWNERCHECK
```
#### SSI
```
AddOutputFilter FILEOWNERCHECK;INCLUDES .shtml
```
## License
under the MIT License:

* http://www.opensource.org/licenses/mit-license.php
