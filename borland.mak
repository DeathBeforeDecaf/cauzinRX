#
# Borland C++ IDE generated makefile
#
.AUTODEPEND


#
# Borland C++ tools
#
IMPLIB  = Implib
BCCDOS  = Bcc +BccDos.cfg 
TLINK   = TLink
TLIB    = TLib
TASM    = Tasm
#
# IDE macros
#


#
# Options
#
IDE_LFLAGSDOS =  -LC:\PROGRAMS\BCPP\LIB
IDE_BFLAGS = 
LLATDOS_cauzinrxdexe =  -LC:\PROGRAMS\BCPP\LIB;C:\PROGRAMS\BCPP\LIB -c -Tde
RLATDOS_cauzinrxdexe = 
BLATDOS_cauzinrxdexe = 
CNIEAT_cauzinrxdexe = -IC:\PROGRAMS\BCPP\INCLUDE;C:\PROGRAMS\BCPP\INCLUDE -D
LNIEAT_cauzinrxdexe = -x
LEAT_cauzinrxdexe = $(LLATDOS_cauzinrxdexe)
REAT_cauzinrxdexe = $(RLATDOS_cauzinrxdexe)
BEAT_cauzinrxdexe = $(BLATDOS_cauzinrxdexe)

#
# Dependency List
#
Dep_cauzinrx = \
   cauzinrx.exe

cauzinrx : BccDos.cfg $(Dep_cauzinrx)
  echo MakeNode 

Dep_cauzinrxdexe = \
   cauzinrx.obj\
   crc16.obj\
   rxbittst.obj\
   rxcaptur.obj\
   rxinspct.obj\
   rxseqnce.obj\
   rxtermnl.obj\
   serlib.obj\
   support.obj\
   syscfg.obj

cauzinrx.exe : $(Dep_cauzinrxdexe)
  $(TLINK)   @&&|
 /v $(IDE_LFLAGSDOS) $(LEAT_cauzinrxdexe) $(LNIEAT_cauzinrxdexe) +
C:\PROGRAMS\BCPP\LIB\c0l.obj+
cauzinrx.obj+
crc16.obj+
rxbittst.obj+
rxcaptur.obj+
rxinspct.obj+
rxseqnce.obj+
rxtermnl.obj+
serlib.obj+
support.obj+
syscfg.obj
$<,$*
C:\PROGRAMS\BCPP\LIB\bidsl.lib+
C:\PROGRAMS\BCPP\LIB\fp87.lib+
C:\PROGRAMS\BCPP\LIB\mathl.lib+
C:\PROGRAMS\BCPP\LIB\cl.lib

|

cauzinrx.obj :  src\cauzinrx.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_cauzinrxdexe) $(CNIEAT_cauzinrxdexe) -o$@ src\cauzinrx.c
|

crc16.obj :  src\strprx16\crc16.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_cauzinrxdexe) $(CNIEAT_cauzinrxdexe) -o$@ src\strprx16\crc16.c
|

rxbittst.obj :  src\strprx16\rxbittst.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_cauzinrxdexe) $(CNIEAT_cauzinrxdexe) -o$@ src\strprx16\rxbittst.c
|

rxcaptur.obj :  src\strprx16\rxcaptur.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_cauzinrxdexe) $(CNIEAT_cauzinrxdexe) -o$@ src\strprx16\rxcaptur.c
|

rxinspct.obj :  src\strprx16\rxinspct.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_cauzinrxdexe) $(CNIEAT_cauzinrxdexe) -o$@ src\strprx16\rxinspct.c
|

rxseqnce.obj :  src\strprx16\rxseqnce.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_cauzinrxdexe) $(CNIEAT_cauzinrxdexe) -o$@ src\strprx16\rxseqnce.c
|

rxtermnl.obj :  src\strprx16\rxtermnl.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_cauzinrxdexe) $(CNIEAT_cauzinrxdexe) -o$@ src\strprx16\rxtermnl.c
|

serlib.obj :  src\strprx16\serlib.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_cauzinrxdexe) $(CNIEAT_cauzinrxdexe) -o$@ src\strprx16\serlib.c
|

support.obj :  src\strprx16\support.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_cauzinrxdexe) $(CNIEAT_cauzinrxdexe) -o$@ src\strprx16\support.c
|

syscfg.obj :  src\strprx16\syscfg.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_cauzinrxdexe) $(CNIEAT_cauzinrxdexe) -o$@ src\strprx16\syscfg.c
|

# Compiler configuration file
BccDos.cfg : 
   Copy &&|
-W-
-R
-v
-vi
-H
-H=cauzinrx.csm
-ml
-f287
| $@


