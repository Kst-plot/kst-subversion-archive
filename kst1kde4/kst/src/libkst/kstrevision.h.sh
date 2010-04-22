#!/bin/sh

if (which svn >/dev/null 2>&1); then
    if (svn info $1 >/dev/null 2>&1); then
        REV=`svn info $1 | grep ^Revision | cut -b 11-`;
    else REV="dist"; fi; 
else REV="dist"; fi;

echo "#ifndef KSTREVISION" >kstrevision.h.tmp;
echo "#define KSTREVISION \""$REV"\"" >>kstrevision.h.tmp;
echo "#endif" >>kstrevision.h.tmp;

if (diff -N kstrevision.h.tmp kstrevision.h >/dev/null 2>&1);
    then rm kstrevision.h.tmp; 
    else mv kstrevision.h.tmp kstrevision.h; fi
