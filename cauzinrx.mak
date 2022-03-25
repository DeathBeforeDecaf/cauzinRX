# Microsoft Visual C++ generated build script - Do not modify

PROJ = CAUZINRX
DEBUG = 0
PROGTYPE = 6
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\PROJECTS\OUTBOX\CAUZINRX\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = CAUZINRX.C  
FIRSTCPP =             
RC = rc
CFLAGS_D_DEXE = /nologo /G3 /FPc /W3 /Zi /AL /Od /D "_DEBUG" /D "_DOS" /FR /Fd"CAUZINRX.PDB"
CFLAGS_R_DEXE = /nologo /Gs /G3 /FPc /W3 /AL /Ox /D "NDEBUG" /D "_DOS" /FR 
LFLAGS_D_DEXE = /NOLOGO /NOI /STACK:5120 /ONERROR:NOEXE /CO 
LFLAGS_R_DEXE = /NOLOGO /NOI /STACK:5120 /ONERROR:NOEXE 
LIBS_D_DEXE = oldnames llibce 
LIBS_R_DEXE = oldnames llibce 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_DEXE)
LFLAGS = $(LFLAGS_D_DEXE)
LIBS = $(LIBS_D_DEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_DEXE)
LFLAGS = $(LFLAGS_R_DEXE)
LIBS = $(LIBS_R_DEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = CAUZINRX.SBR \
		SERLIB.SBR \
		SUPPORT.SBR \
		RXSEQNCE.SBR \
		CRC16.SBR \
		RXINSPCT.SBR \
		SYSCFG.SBR \
		RXTERMNL.SBR \
		RXBITTST.SBR \
		RXCAPTUR.SBR


CAUZINRX_DEP = src/strprx16/stdtypes.h \
	src/strprx16/support.h \
	src/strprx16/cznstrip.h \
	src/strprx16/serlib.h \
	src/strprx16/stdtypes.h \
	src/strprx16/syscfg.h \
	src/strprx16/support.h


SERLIB_DEP = src/strprx16/serlib.h \
	src/strprx16/stdtypes.h


SUPPORT_DEP = src/strprx16/support.h \
	src/strprx16/cznstrip.h \
	src/strprx16/serlib.h \
	src/strprx16/stdtypes.h \
	src/strprx16/syscfg.h


RXSEQNCE_DEP = src/strprx16/crc16.h \
	src/strprx16/stdtypes.h \
	src/strprx16/cznstrip.h \
	src/strprx16/serlib.h \
	src/strprx16/support.h \
	src/strprx16/syscfg.h


CRC16_DEP = src/strprx16/crc16.h \
	src/strprx16/stdtypes.h


RXINSPCT_DEP = src\strprx16\support.h \
	src/strprx16/cznstrip.h \
	src/strprx16/serlib.h \
	src/strprx16/stdtypes.h \
	src/strprx16/syscfg.h


SYSCFG_DEP = src/strprx16/syscfg.h \
	src/strprx16/stdtypes.h \
	src/strprx16/support.h


RXTERMNL_DEP = src/strprx16/cznstrip.h \
	src/strprx16/serlib.h \
	src/strprx16/stdtypes.h \
	src/strprx16/support.h \
	src/strprx16/syscfg.h


RXBITTST_DEP = src/strprx16/cznstrip.h \
	src/strprx16/serlib.h \
	src/strprx16/stdtypes.h \
	src/strprx16/support.h \
	src/strprx16/syscfg.h


RXCAPTUR_DEP = src/strprx16/cznstrip.h \
	src/strprx16/serlib.h \
	src/strprx16/stdtypes.h \
	src/strprx16/support.h \
	src/strprx16/syscfg.h


all:    $(PROJ).EXE $(PROJ).BSC

CAUZINRX.OBJ:   SRC/CAUZINRX.C $(CAUZINRX_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c SRC/CAUZINRX.C

SERLIB.OBJ:     SRC/STRPRX16/SERLIB.C $(SERLIB_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SRC/STRPRX16/SERLIB.C

SUPPORT.OBJ:    SRC/STRPRX16/SUPPORT.C $(SUPPORT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SRC/STRPRX16/SUPPORT.C

RXSEQNCE.OBJ:   SRC/STRPRX16/RXSEQNCE.C $(RXSEQNCE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SRC/STRPRX16/RXSEQNCE.C

CRC16.OBJ:      SRC/STRPRX16/CRC16.C $(CRC16_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SRC/STRPRX16/CRC16.C

RXINSPCT.OBJ:   SRC/STRPRX16/RXINSPCT.C $(RXINSPCT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SRC/STRPRX16/RXINSPCT.C

SYSCFG.OBJ:     SRC/STRPRX16/SYSCFG.C $(SYSCFG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SRC/STRPRX16/SYSCFG.C

RXTERMNL.OBJ:   SRC/STRPRX16/RXTERMNL.C $(RXTERMNL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SRC/STRPRX16/RXTERMNL.C

RXBITTST.OBJ:   SRC/STRPRX16/RXBITTST.C $(RXBITTST_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SRC/STRPRX16/RXBITTST.C

RXCAPTUR.OBJ:   SRC/STRPRX16/RXCAPTUR.C $(RXCAPTUR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SRC/STRPRX16/RXCAPTUR.C

$(PROJ).EXE::   CAUZINRX.OBJ SERLIB.OBJ SUPPORT.OBJ RXSEQNCE.OBJ CRC16.OBJ RXINSPCT.OBJ \
	SYSCFG.OBJ RXTERMNL.OBJ RXBITTST.OBJ RXCAPTUR.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
CAUZINRX.OBJ +
SERLIB.OBJ +
SUPPORT.OBJ +
RXSEQNCE.OBJ +
CRC16.OBJ +
RXINSPCT.OBJ +
SYSCFG.OBJ +
RXTERMNL.OBJ +
RXBITTST.OBJ +
RXCAPTUR.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
