#!/usr/bin/env tmux-script

session mcu-py

window vim test.c
rename test

window
rename run

window vim core.h
rename core
vsplit vim core.c

window vim execute.h
rename execute
vsplit vim execute.c

window vim globals.h
rename globals
vsplit vim globals.c

window vim error.h
rename error
vsplit vim error.c

window vim tables.c
rename tables

window vim debug.h
rename debug
vsplit vim debug.c

window vim blog.txt
rename texts
vsplit vim notes.txt

window vim ~/projects/python/py-temp/temp.py
rename python
vsplit

