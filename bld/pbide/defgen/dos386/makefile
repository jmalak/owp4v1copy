#pmake: os_dos cpu_386

proj_name = defgen

host_OS  = dos
host_CPU = 386

xname = defgen.exe

lnk = $(name).lnk

!include cproj.mif
!include deftarg.mif
!include defrule.mif

inc_dirs = -I..

objs = ytab.obj main.obj scan.obj error.obj semantic.obj output.obj

$(xname) : ytab.h ytab.c $(objs) $(__MAKEFILES__)
        $(cl) $(clflags) $(objs) $(cl_libs)

.c: ..

ytab.c ytab.h: ../yydriver.c ../defgen.y
        $(byacc) -d $]@ $[@

additional_cleanup = ytab.h ytab.c
