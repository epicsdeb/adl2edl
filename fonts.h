#ifndef __fonts_h
#define __fonts_h 1

#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <Xm/Xm.h>
#include <math.h>

#define FONTINFO_SUCCESS 1
#define FONTINFO_EMPTY 100
#define FONTINFO_FAIL 102
#define FONTINFO_NO_FILE 104
#define FONTINFO_NO_FONT 106
#define FONTINFO_NO_MEM 108
#define FONTINFO_SYNTAX 110
#define FONTINFO_BADSPEC 112
#define FONTINFO_TOOMANYSIZES 114
#define FONTINFO_MISSINGBRACE 116
#define FONTINFO_GROUPSYNTAX 118
#define FONTINFO_UNSUPPORTED 120

#define fontInfoClass_str1 (char *) "Error from avl_get_first\n"
#define fontInfoClass_str2 (char *) "Error from avl_get_next\n"
#define fontInfoClass_str3 (char *) "Expected default site font near line %-d\n"
#define fontInfoClass_str4 (char *) "Expected default text create font near line %-d\n"
#define fontInfoClass_str5 (char *) "Font file syntax error near line %-d\n"
#define fontInfoClass_str6 (char *) "No fonts were specified\n"
#define fontInfoClass_str7 (char *) "To many font sizes at line %-d\n"
#define fontInfoClass_str8 (char *) "No such font available: [%s]\n"
#define fontInfoClass_str9 (char *) "Last line processed: %-d\n"
#define fontInfoClass_str10 (char *) "Missing \"}\" in font group definition\n"
#define fontInfoClass_str11 (char *) "Internal error at line %-d in file %s\n"
#define fontInfoClass_str12 (char *) "Font group syntax error near line %-d\n"
#define fontInfoClass_str13 (char *) "Warning - duplicate font tag name: [%s], from line: %-d\n"

typedef struct fontNameListTag {
  struct fontNameListTag *flink;
  XFontStruct *fontStruct;
  char *fullName;
  char *name;
  char *family;
  int size;
  float fsize;
  char weight;   // m or b
  char slant;    // r or i
  char isScalable;
  char fontLoaded;
  int ascent;
  int descent;
  int height;
} fontNameListType, *fontNameListPtr;

typedef struct sizeListTag {
  struct sizeListTag *flink;
  int size;
  float fsize;
} sizeListType, *sizeListPtr;

typedef struct familyListTag {
  struct familyListTag *flink;
  sizeListPtr sizeHead;
  sizeListPtr sizeTail;
  char *name;
} familyListType, *familyListPtr;

class fontInfoClass {

public:

fontInfoClass ( void );   // constructor

~fontInfoClass ( void );   // destructor

int InitializeXt( void );

char *getStrFromFile (
  char *str,
  int maxLen,
  FILE *f
);

int parseFontSpec (
  char *fontSpec,
  char *foundary,
  char *family,
  char *weight,
  char *slant,
  char *pixelSize );

int resolveFont (
  char *fontSpec,
  fontNameListPtr ptr );

void setMediumString (
  char *str
);

void setBoldString (
  char *str
);

void setRegularString (
  char *str
);

void setItalicString (
  char *str
);

int resolveFont (
  char *fontSpec,
  char *userFontFamilyName,
  fontNameListPtr ptr );

int resolveOneFont (
  char *fontSpec,
  fontNameListPtr ptr );

int checkSingleFontSpecGeneric (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *line,
  int checkBestFont,
  int major,
  int minor,
  int release );

int checkSingleFontSpec (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *line,
  int major,
  int minor,
  int release );

int checkBestSingleFontSpec (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *line,
  int major,
  int minor,
  int release );

int getSingleFontSpec (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *buf,
  int major,
  int minor,
  int release );

int flushToBrace (
  FILE *f );

int processFontGroup (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  FILE *f,
  int major,
  int minor,
  int release );

int initFromFileVer3 (
  XtAppContext app,
  Display *d,
  FILE *f,
  int major,
  int minor,
  int release );

int initFromFile (
  XtAppContext app,
  Display *d,
  const char *fileName );

XFontStruct *getXFontStruct (
  char *name );

XmFontList getXmFontList ( void );

int loadFontTag (
  char *name );

int getTextFontList (
  char *name,
  XmFontList *fontList );

int textWidth (
  char *name,
  char *string );

char *bestFittingFont (
  int height );

private:

int initOK;

XmFontList fontList;
int fontListEmpty;

char mediumString[63+1], boldString[63+1], regularString[63+1],
 italicString[63+1];

int lineNum, lastNonCommentLine;

int requireExactMatch;

Display *display;
XtAppContext appCtx;

};

extern	const char * fontFileName;
#endif
