#ifndef _MIME_H_
#define _MIME_H_

typedef struct mime {
    char *file_extension;
    char *http_type;
    mime_t *next;
} mime_t;

/* Initialize MIME type*/
mime_t MIME[] = {{".ez", "application/andrew-inset"},
                 {".aw", "application/applixware"},
                 {".atom", "application/atom+xml"},
                 {".atomcat", "application/atomcat+xml"},
                 {".atomsvc", "application/atomsvc+xml"},
                 {".ccxml", "application/ccxml+xml"},
                 {".cu", "application/cu-seeme"},
                 {".davmount", "application/davmount+xml"},
                 {".ecma", "application/ecmascript"},
                 {".emma", "application/emma+xml"},
                 {".epub", "application/epub+zip"},
                 {".pfr", "application/font-tdpfr"},
                 {".gz", "application/gzip"},
                 {".tgz", "application/gzip"},
                 {".stk", "application/hyperstudio"},
                 {".jar", "application/java-archive"},
                 {".ser", "application/java-serialized-object"},
                 {".class", "application/java-vm"},
                 {".json", "application/json"},
                 {".lostxml", "application/lost+xml"},
                 {".hqx", "application/mac-binhex40"},
                 {".cpt", "application/mac-compactpro"},
                 {".mrc", "application/marc"},
                 {".ma", "application/mathematica"},
                 {".mb", "application/mathematica"},
                 {".nb", "application/mathematica"},
                 {".mathml", "application/mathml+xml"},
                 {".mml", "application/mathml+xml"},
                 {".mbox", "application/mbox"},
                 {".mscml", "application/mediaservercontrol+xml"},
                 {".mp4s", "application/mp4"},
                 {".doc", "application/msword"},
                 {".dot", "application/msword"},
                 {".mxf", "application/mxf"},
                 {".a", "application/octet-stream"},
                 {".bin", "application/octet-stream"},
                 {".bpk", "application/octet-stream"},
                 {".o", "application/octet-stream"},
                 {".obj", "application/octet-stream"},
                 {".pkg", "application/octet-stream"},
                 {".so", "application/octet-stream"},
                 {".iso", "application/octet-stream"},
                 {".dmg", "application/octet-stream"},
                 {".elc", "application/octet-stream"},
                 {".oda", "application/oda"},
                 {".opf", "application/oebps-package+xml"},
                 {".ogx", "application/ogg"},
                 {".onepkg", "application/onenote"},
                 {".xer", "application/patch-ops-error+xml"},
                 {".pdf", "application/pdf"},
                 {".pgp", "application/pgp-encrypted"},
                 {".pki", "application/pkixcmp"},
                 {".xml", "application/xml"},
                 {".tar", "application/x-tar"},
                 {".sh", "application/x-shellscript"},
                 {".pyc", "application/x-python-code"},
                 {".rpa", "application/x-redhat-package-manager"},
                 {".bmp", "image/bmp"},
                 {".cgm", "image/cgm"},
                 {".gif", "image/gif"},
                 {".jpe", "image/jpeg"},
                 {".jpeg", "image/jpeg"},
                 {".jpg", "image/jpeg"},
                 {".pjpg", "image/jpeg"},
                 {".png", "image/png"},
                 {".ras", "image/x-cmu-raster"},
                 {".c", "text/x-c"},
                 {".cc", "text/x-c"},
                 {".h", "text/x-c"},
                 {".cpp", "text/x-c"}}
#endif