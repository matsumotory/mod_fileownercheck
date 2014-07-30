# mod_fileownercheck

mod_fileownercheck checks between owner of opened r->filename and that of current r->filename at output filter phase. This module resolves TOCTOU with FollowSymlinks and checks a permission of static contensts on VirtualHost.

ref. [Apache does not honor -FollowSymlinks due to TOCTOU](https://bugs.launchpad.net/ubuntu/+source/apache2/+bug/811428)

## How to Use
### Quick Install
```
apxs -c -i mod_fileownercheck.c
```
### Config
#### Load Module
```apache
LoadModule fileownercheck_module modules/mod_fileownercheck.so
```
#### Enable suEXEC Check

Set Enable Owner Check Using ``SuexecUserGgroup`` config (On / Off default Off).
If ``FOCSuexecEnable On``, mod_fileowner checks between a owner of opened ``r->filename`` and a user configured by ``SuexecUserGroup``.

```apache
<Directory /var/www/html/vhost/*/htdocs>
  FOCSuexecEnable On
</Directory>
```

## License
under the MIT License:

* http://www.opensource.org/licenses/mit-license.php
