# mod_fileownercheck

mod_fileownercheck check between owner of opened r->filename and that of current r->filename at output filter phase. This module resolve TOCTOU with FollowSymlinks.

ref. [Apache does not honor -FollowSymlinks due to TOCTOU](https://bugs.launchpad.net/ubuntu/+source/apache2/+bug/811428)

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
