LDMakeList
Use -? for help, -v for version
Valid options are:
-d or -D : sort by _D_escription (default)
-n or -N : sort by file_N_ame
-p or -P : sort by description (_P_atterns at end of file)
-s or -S : use _S_hort descriptions*
-m or -M : include pri_M_itives in the list
-u or -U : include _U_nofficial part directory
-a or -A : exclude _A_liases from the list
-r or -R : _R_emove duplicate entries (experimental)
-o or -O : Include _o_fficial parts only
-l[DIRNAME] or -L[DIRNAME] : Read from _L_Draw director DIRNAME
-h[C] or -H[C] : -H_ide descriptions starting with character(s) [C]
-i[C] or -I[C] : str_I_p leading character(s) [C] from descriptions
-x or -X : do not write parts._x_ml file

eg. "LDMakeList -d -h=_ -i~" will generate a parts.lst file sorted
by description (-d), excluding colour = and some parts _ (-h=_),
and removing the ~ character (-i~) from the start of descriptions

The -L tag lets you select the base directory for your LDraw
installation (defaults to environment variable LDRAWDIR)
eg. "LDMakeList -d -L." sorts by descriptions in the present
directory

eg. "LDMakeList -u" will look in [LDRAWDIR]/Unofficial/Parts
as well as [LDRAWDIR]/Parts for part files

The -m tag is not recommended for beginner LDraw users

* The old limit for part descriptions is 64 characters and it is
possible that descriptions longer than this might break some
old software. This ensures that the output filenames are no
longer than 64 characters. Use if programs are giving errors.
