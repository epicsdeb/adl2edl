#include <stdio.h>
#include "fonts.h"

static fontNameListPtr head, tail;

static int getMatch (
  char *name,
  fontNameListPtr *node )
{

fontNameListPtr cur;

  cur = head->flink;
  while ( cur ) {
    if ( strcmp( name, cur->name ) == 0 ) {
      *node = cur;
      return 1;
    }
    cur = cur->flink;
  }

  *node = NULL;
  return 0;

}

static char *Strncat(
  char *dest,
  const char *src,
  int max ) {

  // max must be >= 0 and no more than stringsize - 1
  //
  // for char string[10];       max must be <= 9

int l, newMax;
char *s;

  l = strlen( dest );
  newMax = max - l;
  if ( newMax < 0 ) {
    dest[max] = 0;
    return dest;
  }

  s = strncat( dest, src, newMax );
  dest[max] = 0;

  return s;

}

fontInfoClass::fontInfoClass ( void ) {   // constructor

int stat, l, h;
int argc = 0;
fontNameListPtr cur;

  initOK = 0;

  fontListEmpty = 1;

  requireExactMatch = 1;

  strcpy( mediumString, "medium" );
  strcpy( boldString, "bold" );
  strcpy( regularString, "r" );
  strcpy( italicString, "i" );

  lineNum = lastNonCommentLine = 1;

  XtToolkitInitialize();
  appCtx = XtCreateApplicationContext();
  display = XtOpenDisplay( appCtx, "", "fonts", "fonts", NULL, 0,
   &argc, NULL );

  head = new fontNameListType;
  tail = head;
  tail->flink = NULL;

  stat = initFromFile( appCtx, display, "fonts.adl2edl" );
  if ( stat & 1 ) initOK = 1;

}

fontInfoClass::~fontInfoClass ( void ) {   // destructor

}

char *fontInfoClass::getStrFromFile (
  char *str,
  int maxLen,
  FILE *f
) {

char *ctx, *ptr, *tk, stackBuf[255+1];
char *buf;
int tryAgain, bufOnHeap;

  // ignore blank lines and comment lines

  if ( maxLen < 1 ) return (char *) NULL;

  if ( maxLen > 255 ) {
    buf = new char[maxLen+1];
    bufOnHeap = 1;
  }
  else {
    buf = stackBuf;
    bufOnHeap = 0;
  }

  do {

    tryAgain = 0;

    ptr = fgets( str, maxLen, f );
    if ( !ptr ) {
      strcpy( str, "" );
      if ( bufOnHeap ) delete [] buf;
      return (char *) NULL;
    }

    lineNum++;

    strcpy( buf, str );

    ctx = NULL;
    tk = strtok_r( buf, "\n", &ctx );

    if ( tk ) {

      if ( tk[0] == '#' ) tryAgain = 1;

    }
    else {

      tryAgain = 1;

    }

  } while ( tryAgain );

  lastNonCommentLine = lineNum;

  if ( bufOnHeap ) delete [] buf;

  return str;

}

int fontInfoClass::parseFontSpec (
  char *fontSpec,
  char *foundary,
  char *family,
  char *weight,
  char *slant,
  char *pixelSize ) {

static const int GETTING_DASH = 1;
static const int GETTING_STRING = 2;
static const int GETTING_LAST_STRING = 3;
static const int STORE_VALUE = 4;
static const int DONE = -1;
int l, ii, iii, i = 0, first = 0, last = 0, n = 0;
int state = GETTING_DASH;
char value[14][63+1];

  l = strlen( fontSpec );

  while ( state != DONE ) {

    //printf( "s: %-d, n=%-d, i=%-d\n", state, n, i );

    switch ( state ) {

    case GETTING_DASH:

      if ( fontSpec[i] == '-' ) {
        i++;
        if ( i >= l ) return FONTINFO_BADSPEC;
        first = i;
        state = GETTING_STRING;
      }
      else if ( fontSpec[i] == '\t' ) {
        return FONTINFO_BADSPEC;
      }
      else {
        return FONTINFO_BADSPEC;
      }

      break;

    case GETTING_STRING:

      if ( fontSpec[i] == '-' ) {
        last = i - 1;
        state = STORE_VALUE;
      }
      else if ( fontSpec[i] == '\t' ) {
        return FONTINFO_BADSPEC;
      }

      i++;
      if ( i >= l ) return FONTINFO_BADSPEC;

      break;

    case GETTING_LAST_STRING:

      if ( fontSpec[i] == '\t' ) {
        last = i - 1;
        state = STORE_VALUE;
      }

      i++;
      if ( i >= l ) {
        last = i - 1;
        state = STORE_VALUE;
      }

      break;

    case STORE_VALUE:

      if ( last >= first ) {

        for ( ii=first, iii=0; ii<=last; ii++, iii++ ) {
          value[n][iii] = fontSpec[ii];
        }
        value[n][iii] = 0;

        //printf( "value[%-d] = [%s]\n", n, value[n] );

      }
      else {

        strcpy( value[n], "" );
        //printf( "value[%-d] = NULL\n", n );

      }

      first = i;

      n++;

      if ( n < 13 ) {
        state = GETTING_STRING;
      }
      else if ( n == 13 ) {
        state = GETTING_LAST_STRING;
      }
      else {
        state = DONE;
      }

      break;

    }

  }

  strncpy( foundary, value[0], 63 );
  foundary[63] = 0;

  strncpy( family, value[1], 63 );
  family[63] = 0;

  strncpy( weight, value[2], 63 );
  weight[63] = 0;

  strncpy( slant, value[3], 63 );
  slant[63] = 0;

  strncpy( pixelSize, value[7], 63 );
  pixelSize[63] = 0;

  return FONTINFO_SUCCESS;

}

static char **findBestFont(
  Display *d,
  char *fontSpec,
  int *n ) {

char **list;
char *tk, spec[127+1], rest[127+1], foundry[63+1], family[63+1], weight[31+1],
 slant[31+1], ftype[31+1], size[31+1], newFont[127+1];

  strncpy( spec, fontSpec, 127 );

  tk = strtok( spec, "-" );
  if ( !tk ) goto err_return;
  strncpy( foundry, tk, 63 );

  tk = strtok( NULL, "-" );
  if ( !tk ) goto err_return;
  strncpy( family, tk, 63 );

  tk = strtok( NULL, "\n" );
  if ( !tk ) goto err_return;
  strncpy( rest, tk, 127 );

  strncpy( newFont, "-", 127 );
  Strncat( newFont, foundry, 127 );
  Strncat( newFont, "-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, rest, 127 );

  //  printf( "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( spec, rest, 127 );

  tk = strtok( spec, "-" );
  if ( !tk ) goto err_return;
  strncpy( weight, tk, 31 );

  tk = strtok( NULL, "\n" );
  if ( !tk ) goto err_return;
  strncpy( rest, tk, 127 );

  strncpy( newFont, "-", 127 );
  Strncat( newFont, foundry, 127 );
  Strncat( newFont, "-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, rest, 127 );

//   printf( "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( spec, rest, 127 );

  tk = strtok( spec, "-" );
  if ( !tk ) goto err_return;
  strncpy( slant, tk, 31 );

  tk = strtok( NULL, "\n" );
  if ( !tk ) goto err_return;
  strncpy( rest, tk, 127 );

  strncpy( newFont, "-", 127 );
  Strncat( newFont, foundry, 127 );
  Strncat( newFont, "-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, rest, 127 );

//   printf( "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( spec, rest, 127 );

  tk = strtok( spec, "-" );
  if ( !tk ) goto err_return;
  strncpy( ftype, tk, 31 );

  tk = strtok( NULL, "-" );
  if ( !tk ) goto err_return;
  strncpy( size, tk, 31 );

  tk = strtok( NULL, "\n" );
  if ( !tk ) goto err_return;
  strncpy( rest, tk, 127 );

  strncpy( newFont, "-", 127 );
  Strncat( newFont, foundry, 127 );
  Strncat( newFont, "-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, ftype, 127 );
  Strncat( newFont, "--", 127 );
  Strncat( newFont, size, 127 );
  Strncat( newFont, "-", 127 );
  Strncat( newFont, "*-*-*-*-*-*-*", 127 );

//   printf( "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( newFont, "-", 127 );
  Strncat( newFont, foundry, 127 );
  Strncat( newFont, "-*-*-*-*--*-*-*-*-*-*-*-*", 127 );

//   printf( "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( newFont, "-*-*-*-*-*--*-*-*-*-*-*-*-*", 127 );

//   printf( "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

err_return:
  *n = 0;
  return (char **) NULL;

}

int fontInfoClass::resolveFont (
  char *fontSpec,
  fontNameListPtr ptr ) {

int n, isize, isScalable;
float fsize;
char **list;
char *tk, spec[127+1], name[127+1], family[63+1], weight[31+1],
 slant[31+1], size[31+1];

  ptr->fontLoaded = 0;

  list = XListFonts( this->display, fontSpec, 1, &n );
  if ( n == 0 ) {
    list = findBestFont( this->display, fontSpec, &n );
    if ( n == 0 ) {
      return FONTINFO_NO_FONT;
    }
  }

  strncpy( spec, list[0], 127 );

  tk = strtok( spec, "-" );

  tk = strtok( NULL, "-" );
  strncpy( family, tk, 63 );

  tk = strtok( NULL, "-" );
  strncpy( weight, tk, 31 );

  tk = strtok( NULL, "-" );
  if ( strcmp( tk, "r" ) == 0 )
    strncpy( slant, "r", 31 );
  else
    strncpy( slant, "i", 31 );

  tk = strtok( NULL, "-" );
  tk = strtok( NULL, "-" );

  tk = strtok( NULL, "-" );
  strncpy( size, tk, 31 );
  if ( strcmp( size, "0" ) == 0 )
    isScalable = 1;
  else
    isScalable = 0;

  isize = atol( size );
  fsize = atof( size );
  fsize /= 10;
  ptr->size = isize;
  ptr->fsize = fsize;

  sprintf( size, "%-.1f", fsize );

  strncpy( name, family, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, weight, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, slant, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, size, 127 );

  ptr->isScalable = (char) isScalable;

  ptr->fullName = new char[strlen(list[0])+1];
  strcpy( ptr->fullName, list[0] );

  ptr->name = new char[strlen(name)+1];
  strcpy( ptr->name, name );

  ptr->family = new char[strlen(family)+1];
  strcpy( ptr->family, family );

  ptr->weight = weight[0];

  ptr->slant = slant[0];

  XFreeFontNames( list );

  return FONTINFO_SUCCESS;

}

void fontInfoClass::setMediumString (
  char *str
) {

  strncpy( mediumString, str, 63 );
  mediumString[63] = 0;

}
void fontInfoClass::setBoldString (
  char *str
) {

  strncpy( boldString, str, 63 );
  boldString[63] = 0;

}
void fontInfoClass::setRegularString (
  char *str
) {

  strncpy( regularString, str, 63 );
  regularString[63] = 0;

}
void fontInfoClass::setItalicString (
  char *str
) {

  strncpy( italicString, str, 63 );
  italicString[63] = 0;

}

int fontInfoClass::resolveFont (
  char *fontSpec,
  char *userFontFamilyName,
  fontNameListPtr ptr ) {

int n, isize, isScalable, stat;
float fsize;
char **list;
char spec[127+1], name[127+1], foundary[63+1], family[63+1], weight[63+1],
 slant[63+1], size[63+1];

  ptr->fontLoaded = 0;

  list = XListFonts( this->display, fontSpec, 1, &n );
  if ( n == 0 ) {
    if ( requireExactMatch ) {
      //printf( fontInfoClass_str8, fontSpec );
      //printf( fontInfoClass_str9, lastNonCommentLine );
      return FONTINFO_NO_FONT;
    }
    else {
      list = findBestFont( this->display, fontSpec, &n );
      if ( n == 0 ) {
        //printf( fontInfoClass_str8, fontSpec );
        //printf( fontInfoClass_str9, lastNonCommentLine );
        return FONTINFO_NO_FONT;
      }
    }
  }

  strncpy( spec, list[0], 127 );

  //printf( "Font Spec: [%s]\n", spec );

  stat = parseFontSpec( spec, foundary, family, weight, slant, size );

  if ( strcmp( weight, mediumString ) == 0 ) {
    strcpy( weight, "medium" );
  }
  else if ( strcmp( weight, boldString ) == 0 ) {
    strcpy( weight, "bold" );
  }
  else {
    strcpy( weight, "medium" );
  }

  if ( strcmp( slant, regularString ) == 0 ) {
    strcpy( slant, "r" );
  }
  else if ( strcmp( slant, italicString ) == 0 ) {
    strcpy( slant, "i" );
  }
  else {
    strcpy( slant, "r" );
  }

  if ( strcmp( size, "0" ) == 0 )
    isScalable = 1;
  else
    isScalable = 0;

  isize = atol( size );
  fsize = atof( size );
  fsize /= 10;
  ptr->size = isize;
  ptr->fsize = fsize;

  sprintf( size, "%-.1f", fsize );

  strncpy( name, userFontFamilyName, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, weight, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, slant, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, size, 127 );

  //printf( "name=[%s]\n", name );

  ptr->isScalable = (char) isScalable;

  ptr->fullName = new char[strlen(list[0])+1];
  strcpy( ptr->fullName, list[0] );

  ptr->name = new char[strlen(name)+1];
  strcpy( ptr->name, name );

  ptr->family = new char[strlen(userFontFamilyName)+1];
  strcpy( ptr->family, userFontFamilyName );

  ptr->weight = weight[0];

  ptr->slant = slant[0];

  XFreeFontNames( list );

  return FONTINFO_SUCCESS;

}

int fontInfoClass::resolveOneFont (
  char *fontSpec,
  fontNameListPtr ptr ) {

int n, isize, isScalable;
float fsize;
char **list;
char *tk, spec[127+1], name[127+1], family[63+1], weight[31+1],
 slant[31+1], size[31+1];

  ptr->fontLoaded = 0;

  list = XListFonts( this->display, fontSpec, 1, &n );
  if ( n == 0 ) {
    return FONTINFO_NO_FONT;
  }

  strncpy( spec, list[0], 127 );

//   printf( "Spec is [%s]\n", spec );

  tk = strtok( spec, "-" );

  tk = strtok( NULL, "-" );
  strncpy( family, tk, 63 );

  tk = strtok( NULL, "-" );
  strncpy( weight, tk, 31 );

  tk = strtok( NULL, "-" );
  if ( strcmp( tk, "r" ) == 0 )
    strncpy( slant, "r", 31 );
  else
    strncpy( slant, "i", 31 );

  tk = strtok( NULL, "-" );
  tk = strtok( NULL, "-" );

  tk = strtok( NULL, "-" );
  strncpy( size, tk, 31 );
  if ( strcmp( size, "0" ) == 0 )
    isScalable = 1;
  else
    isScalable = 0;

  isize = atol( size );
  fsize = atof( size );
  fsize /= 10;
  ptr->size = isize;
  ptr->fsize = fsize;

  sprintf( size, "%-.1f", fsize );

  strncpy( name, family, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, weight, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, slant, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, size, 127 );

  ptr->isScalable = (char) isScalable;

  ptr->fullName = new char[strlen(list[0])+1];
  strcpy( ptr->fullName, list[0] );

  ptr->name = new char[strlen(name)+1];
  strcpy( ptr->name, name );

  ptr->family = new char[strlen(family)+1];
  strcpy( ptr->family, family );

  ptr->weight = weight[0];

  ptr->slant = slant[0];

  XFreeFontNames( list );

  return FONTINFO_SUCCESS;

}

int fontInfoClass::checkSingleFontSpecGeneric (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *line,
  int checkBestFont,
  int major,
  int minor,
  int release )
{

char buf[255+1], t1[255+1], t2[255+1], t3[255+1], t4[255+1],
 t5[255+1], t6[255+1], t7[255+1], mod[4][255+1], fontSpec[255+1],
 *tk1, *tk2, *ctx1, *ctx2;
int i, ii, iii, n, pointSize[200], numSizes;
int preload;
char **list;

  strncpy( buf, line, 255 );

  tk1 = strtok_r( buf, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t1, tk1 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t2, tk1 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

    // get bold and medium indicators

    ctx2 = NULL;
    tk2 = strtok_r( t2, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[0], tk2 );
    }
    else {
      printf( fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    tk2 = strtok_r( NULL, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[1], tk2 );
    }
    else {
      printf( fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t3, tk1 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t4, tk1 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

    // get italic and regular indicators

    ctx2 = NULL;
    tk2 = strtok_r( t4, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[2], tk2 );
    }
    else {
      printf( fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    tk2 = strtok_r( NULL, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[3], tk2 );
    }
    else {
      printf( fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t5, tk1 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t6, tk1 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

    // get point sizes
    numSizes = 0;
    ctx2 = NULL;
    tk2 = strtok_r( t6, ",", &ctx2 );
    if ( tk2 ) {
      pointSize[numSizes] = atol( tk2 );
      numSizes++;
      if ( numSizes >= 200 ) {
        printf( fontInfoClass_str7, lastNonCommentLine );
        return FONTINFO_TOOMANYSIZES;
      }
    }
    else {
      printf( fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    do {

      tk2 = strtok_r( NULL, ",", &ctx2 );
      if ( tk2 ) {
        pointSize[numSizes] = atol( tk2 );
        numSizes++;
        if ( numSizes >= 200 ) {
          printf( fontInfoClass_str7, lastNonCommentLine );
          return FONTINFO_TOOMANYSIZES;
        }
      }

    } while ( tk2 );

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t7, tk1 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  preload = 0;
  requireExactMatch = 1;

  tk1 = strtok_r( NULL, "\t\n", &ctx1 );
  if ( tk1 ) {
    if ( strcmp( tk1, "preload" ) == 0 ) {
      preload = 1;
    }
    else if ( strcmp( tk1, "exact" ) == 0 ) {
      requireExactMatch = 1;
    }
  }

  tk1 = strtok_r( NULL, "\t\n", &ctx1 );
  if ( tk1 ) {
    if ( strcmp( tk1, "preload" ) == 0 ) {
      preload = 1;
    }
    else if ( strcmp( tk1, "exact" ) == 0 ) {
      requireExactMatch = 1;
    }
  }

  // Build fontspec

  for ( i=0; i<2; i++ ) {

    for ( ii=2; ii<4; ii++ ) {

      for ( iii=0; iii<numSizes; iii++ ) {

        sprintf( fontSpec, "%s%s%s%s%s%-d%s", t1, mod[i], t3, mod[ii],
         t5, pointSize[iii], t7 );

        list = XListFonts( display, fontSpec, 1, &n );
        if ( n == 0 ) {
          if ( checkBestFont && !requireExactMatch ) {
            list = findBestFont( this->display, fontSpec, &n );
            if ( n == 0 ) {
              printf( fontInfoClass_str8, fontSpec );
              printf( fontInfoClass_str9, lastNonCommentLine );
              return FONTINFO_NO_FONT;
            }
          }
          else {
            return FONTINFO_NO_FONT;
          }
        }

        XFreeFontNames( list );

      }

    }

  }

  return FONTINFO_SUCCESS;

}

int fontInfoClass::checkSingleFontSpec (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *line,
  int major,
  int minor,
  int release )
{

int checkBest = 0;

  return checkSingleFontSpecGeneric( app, d, userFontFamilyName,
   line, checkBest, major, minor, release );

}

int fontInfoClass::checkBestSingleFontSpec (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *line,
  int major,
  int minor,
  int release )
{

int checkBest = 1;

  return checkSingleFontSpecGeneric( app, d, userFontFamilyName,
   line, checkBest, major, minor, release );

}

int fontInfoClass::getSingleFontSpec (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *buf,
  int major,
  int minor,
  int release )
{

char t1[255+1], t2[255+1], t3[255+1], t4[255+1],
 t5[255+1], t6[255+1], t7[255+1], mod[4][255+1], fontSpec[255+1],
 *tk1, *tk2, *ctx1, *ctx2;
int i, ii, iii, pointSize[200], numSizes;
int stat, preload;
int empty = 1;
fontNameListPtr cur;
int dup;
XFontStruct *fs;

  tk1 = strtok_r( buf, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t1, tk1 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t2, tk1 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  // get medium and bold modifiers

  ctx2 = NULL;
  tk2 = strtok_r( t2, ",", &ctx2 );
  if ( tk2 ) {
    strcpy( mod[0], tk2 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  setMediumString( mod[0] );

  tk2 = strtok_r( NULL, ",", &ctx2 );
  if ( tk2 ) {
    strcpy( mod[1], tk2 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  setBoldString( mod[1] );

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t3, tk1 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t4, tk1 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  // get italic and regular indicators

  ctx2 = NULL;
  tk2 = strtok_r( t4, ",", &ctx2 );
  if ( tk2 ) {
    strcpy( mod[2], tk2 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  setRegularString( mod[2] );

  tk2 = strtok_r( NULL, ",", &ctx2 );
  if ( tk2 ) {
    strcpy( mod[3], tk2 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  setItalicString( mod[3] );

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t5, tk1 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t6, tk1 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  // get point sizes
  numSizes = 0;
  ctx2 = NULL;
  tk2 = strtok_r( t6, ",", &ctx2 );
  if ( tk2 ) {
    pointSize[numSizes] = atol( tk2 );
    numSizes++;
    if ( numSizes >= 200 ) {
      printf( fontInfoClass_str7, lastNonCommentLine );
      return FONTINFO_TOOMANYSIZES;
    }
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  do {

    tk2 = strtok_r( NULL, ",", &ctx2 );
    if ( tk2 ) {
      pointSize[numSizes] = atol( tk2 );
      numSizes++;
      if ( numSizes >= 200 ) {
        printf( fontInfoClass_str7, lastNonCommentLine );
        return FONTINFO_TOOMANYSIZES;
      }
    }

  } while ( tk2 );

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t7, tk1 );
  }
  else {
    printf( fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  preload = 0;
  requireExactMatch = 1;

  tk1 = strtok_r( NULL, "\t\n", &ctx1 );
  if ( tk1 ) {
    if ( strcmp( tk1, "preload" ) == 0 ) {
      preload = 1;
    }
    else if ( strcmp( tk1, "exact" ) == 0 ) {
      requireExactMatch = 1;
    }
  }

  tk1 = strtok_r( NULL, "\t\n", &ctx1 );
  if ( tk1 ) {
    if ( strcmp( tk1, "preload" ) == 0 ) {
      preload = 1;
    }
    else if ( strcmp( tk1, "exact" ) == 0 ) {
      requireExactMatch = 1;
    }
  }

  // Build fontspec

  for ( i=0; i<1; i++ ) {

    for ( ii=2; ii<3; ii++ ) {

      for ( iii=0; iii<numSizes; iii++ ) {

        sprintf( fontSpec, "%s%s%s%s%s%-d%s", t1, mod[i], t3, mod[ii],
         t5, pointSize[iii], t7 );

        //printf( "[%s]\n", fontSpec );

        cur = new fontNameListType;

        stat = this->resolveFont( fontSpec, userFontFamilyName, cur );
        if ( !( stat & 1 ) ) {
          delete cur;
          return stat;
        }

        tail->flink = cur;
        tail = cur;
        tail->flink = NULL;

        fs = getXFontStruct( cur->name );
        if ( fs ) {
          cur->ascent = fs->ascent;
          cur->descent = fs->descent;
          cur->height = fs->ascent + fs->descent;
        }
        else {
          cur->ascent = 0;
          cur->descent = 0;
          cur->height = 0;
        }

        empty = 0;

      }

    }

  }

  return 1;

}

int fontInfoClass::flushToBrace (
  FILE *f )
{

char line[255+1], *tk, *ptr, *ctx;
int foundBrace;

  do {

    ptr = getStrFromFile ( line, 255, f );
    if ( ptr ) {

      ctx = NULL;

      tk = strtok_r( line, " \t\n", &ctx );
      if ( tk ) {
        if ( tk[0] == '}' ) {
          foundBrace = 1;
        }
        else {
          foundBrace = 0;
        }
      }

    }
    else {
      printf( fontInfoClass_str10 );
      return FONTINFO_FAIL;
    }

  } while ( !foundBrace );

  return FONTINFO_SUCCESS;

}

int fontInfoClass::processFontGroup (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  FILE *f,
  int major,
  int minor,
  int release )
{

char line[255+1], buf[255+1], lastLine[255+1], *ptr, *tk1, *ctx1;
int stat;
int foundBrace;

  strcpy( lastLine, "" );
  stat = FONTINFO_GROUPSYNTAX; // in case all lines are blank

  do {

    ptr = getStrFromFile ( line, 255, f );
    if ( ptr ) {

      ctx1 = NULL;
      strcpy( buf, line );

      tk1 = strtok_r( buf, "\t\n", &ctx1 );
      if ( tk1 ) {
        if ( tk1[0] == '}' ) {
          foundBrace = 1;
        }
        else {
          foundBrace = 0;
        }
      }

      if ( ! foundBrace ) {

        strcpy( lastLine, line );

        stat = checkSingleFontSpec( app, d, userFontFamilyName, line,
         major, minor, release );
        if ( stat & 1 ) {

          //printf( "Using font: %s", line );
          stat = getSingleFontSpec( app, d, userFontFamilyName, line,
           major, minor, release );

          flushToBrace( f );

          return stat; // return stat from getSingleFontSpec

        }
        else {
          //printf( "Font not found: %s", line );
        }

      }

    }
    else {
      printf( fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

  } while ( !foundBrace );

  // If we never found a matching font, try to get something that matches,
  // even badly, using the findBestFont function (in this file)
  stat = checkBestSingleFontSpec( app, d, userFontFamilyName, lastLine,
   major, minor, release );
  if ( stat & 1 ) {
    //printf( "Using font (with substitutions): %s", lastLine );
    stat = getSingleFontSpec( app, d, userFontFamilyName, lastLine,
     major, minor, release );
  }

  if ( stat == FONTINFO_GROUPSYNTAX ) {
    printf( fontInfoClass_str12, lastNonCommentLine );
  }

  return stat; // return last stat from checkSingleFontSpec or GROUPSYNTAX

}

int fontInfoClass::initFromFileVer3 (
  XtAppContext app,
  Display *d,
  FILE *f,
  int major,
  int minor,
  int release )
{

char line[255+1], buf[255+1], userFontFamilyName[63+1], *ptr, *tk1, *ctx1;
int stat;
int empty = 1;

  do {

    ptr = getStrFromFile ( line, 255, f );
    if ( ptr ) {

      empty = 0;

      ctx1 = NULL;
      strcpy( buf, line );

      tk1 = strtok_r( buf, "=\t\n()", &ctx1 );
      if ( tk1 ) {
        strncpy( userFontFamilyName, tk1, 63 );
        userFontFamilyName[63] = 0;
      }
      else {
        fclose( f );
        printf( fontInfoClass_str5, lastNonCommentLine );
        return FONTINFO_SYNTAX;
      }

      tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
      if ( tk1 ) {
        if ( strcmp( tk1, "{" ) == 0 ) { // font groups
          stat = processFontGroup( app, d, userFontFamilyName, f,
           major, minor, release );
          if ( !( stat & 1 ) ) {
            fclose( f );
            return stat;
          }
        }
        else {

          // tk1 points to first character after "<name>="

          strcpy( buf, line );

          stat = getSingleFontSpec( app, d, userFontFamilyName, tk1,
           major, minor, release );
          if ( !( stat & 1 ) ) {
            fclose( f );
            return stat;
          }

        }

      }
      else {
        fclose( f );
        printf( fontInfoClass_str5, lastNonCommentLine );
        return FONTINFO_SYNTAX;
      }

    }

  } while ( ptr );

  fclose( f );

  if ( empty ) {
    printf( fontInfoClass_str6 );
    return FONTINFO_EMPTY;
  }

  return FONTINFO_SUCCESS;

}

int fontInfoClass::initFromFile (
  XtAppContext app,
  Display *d,
  const char *fileName )
{

// Read font specs from given file, query server, and populate data structure.
// If font does not exist, use some other font.

char line[127+1], *ptr, *fontSpec, *tk, *ctx;
int stat, preload;
FILE *f;
int empty = 1;
fontNameListPtr cur;
int dup;
int major, minor, release;
XFontStruct *fs;

  this->display = d;

  f = fopen( fileName, "r" );
  if ( !f ) {
    return FONTINFO_NO_FILE;
  }

  ptr = fgets( line, 127, f );
  if ( !ptr ) {
    printf( fontInfoClass_str6 );
    fclose(f);
    return FONTINFO_EMPTY;
  }

  sscanf( line, "%d %d %d\n", &major, &minor, &release );

  if ( major == 3 ) {
    stat = initFromFileVer3( app, d, f, major, minor, release );
    return stat;
  }

  fclose(f);
  return FONTINFO_UNSUPPORTED;

}

XFontStruct *fontInfoClass::getXFontStruct (
  char *name )
{

int stat;
fontNameListPtr cur;
XmFontListEntry entry;

  stat = getMatch( name, &cur );
  if ( !(stat & 1) ) return (XFontStruct *) NULL;

  if ( !cur ) return (XFontStruct *) NULL;

  if ( cur->fontLoaded ) return cur->fontStruct;


  cur->fontStruct = XLoadQueryFont( this->display, cur->fullName );

  entry = XmFontListEntryLoad( this->display, cur->fullName,
   XmFONT_IS_FONT, cur->name );


  if ( entry ) {


    if ( this->fontListEmpty ) {
      this->fontList = XmFontListAppendEntry( NULL, entry );
      this->fontListEmpty = 0;
    }
    else {
      this->fontList = XmFontListAppendEntry( this->fontList, entry );
    }

    XmFontListEntryFree( &entry );


  }

  if ( cur->fontStruct ) {
    cur->fontLoaded = 1;
  }

  return cur->fontStruct;

}

XmFontList fontInfoClass::getXmFontList ( void )
{

  return this->fontList;

}

int fontInfoClass::loadFontTag (
  char *name )
{

int stat;
fontNameListPtr cur;
XmFontListEntry entry;

  stat = getMatch( name, &cur );
  if ( !(stat & 1) ) return FONTINFO_FAIL;

  if ( !cur ) return FONTINFO_FAIL;

  if ( cur->fontLoaded ) return FONTINFO_SUCCESS;

  cur->fontStruct = XLoadQueryFont( this->display, cur->fullName );

  entry = XmFontListEntryLoad( this->display, cur->fullName,
   XmFONT_IS_FONT, cur->name );


  if ( entry ) {

    if ( this->fontListEmpty ) {
      this->fontList = XmFontListAppendEntry( NULL, entry );
      this->fontListEmpty = 0;
    }
    else {
      this->fontList = XmFontListAppendEntry( this->fontList, entry );
    }

    XmFontListEntryFree( &entry );


  }

  if ( cur->fontStruct ) {
    cur->fontLoaded = 1;
  }

  return FONTINFO_SUCCESS;

}

int fontInfoClass::getTextFontList (
  char *name,
  XmFontList *oneFontList )
{

int stat;
fontNameListPtr cur;
XmFontListEntry entry;

  stat = getMatch( name, &cur );
  if ( !(stat & 1) ) return FONTINFO_FAIL;

  if ( !cur ) return FONTINFO_FAIL;

  if ( !cur->fontLoaded ) {
    stat = this->loadFontTag( name );
    if ( !( stat & 1 ) ) return FONTINFO_FAIL;
  }


  entry = XmFontListEntryLoad( this->display, cur->fullName,
   XmFONT_IS_FONT, cur->name );


  if ( entry ) {

    *oneFontList = XmFontListAppendEntry( NULL, entry );

    XmFontListEntryFree( &entry );


  }
  else {

    return FONTINFO_FAIL;

  }

  return FONTINFO_SUCCESS;

}

char *fontInfoClass::bestFittingFont (
  int height )
{

fontNameListPtr cur;
char *best;

  if ( !initOK ) return NULL;

  cur = head->flink;

  if ( cur ) {
    best = cur->name; // smallest
  }
  else {
    best = NULL;
  }

  while ( cur ) {
    if ( cur->height > height ) {
      return best;
    }
    best = cur->name;
    cur = cur->flink;
  }

  return best;

}

int fontInfoClass::textWidth (
  char *name,
  char *string )
{

XFontStruct *fs;
int l, w;

  if ( !initOK ) return 0;

  if ( string ) {
    l = strlen(string);
  }
  else {
    return 0;
  }

  fs = getXFontStruct( name );
  if ( fs ) {
    return (int) XTextWidth( fs, string, l );
  }
  else {
    return 0;
  }

}
