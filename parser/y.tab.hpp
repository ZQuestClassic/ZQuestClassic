/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SCRIPT = 258,
     FLOAT = 259,
     FOR = 260,
     BOOL = 261,
     VOID = 262,
     IF = 263,
     ELSE = 264,
     RETURN = 265,
     IMPORT = 266,
     TRUE = 267,
     FALSE = 268,
     WHILE = 269,
     FFC = 270,
     ITEM = 271,
     ITEMCLASS = 272,
     GLOBAL = 273,
     ASSIGN = 274,
     SEMICOLON = 275,
     COMMA = 276,
     LBRACKET = 277,
     RBRACKET = 278,
     LPAREN = 279,
     RPAREN = 280,
     DOT = 281,
     LBRACE = 282,
     RBRACE = 283,
     ARROW = 284,
     NUMBER = 285,
     IDENTIFIER = 286,
     QUOTEDSTRING = 287,
     RSHIFT = 288,
     LSHIFT = 289,
     BITXOR = 290,
     BITOR = 291,
     BITAND = 292,
     OR = 293,
     AND = 294,
     BITNOT = 295,
     NOT = 296,
     DECREMENT = 297,
     INCREMENT = 298,
     NE = 299,
     EQ = 300,
     GT = 301,
     GE = 302,
     LT = 303,
     LE = 304,
     MINUS = 305,
     PLUS = 306,
     DIVIDE = 307,
     TIMES = 308,
     MODULO = 309
   };
#endif
/* Tokens.  */
#define SCRIPT 258
#define FLOAT 259
#define FOR 260
#define BOOL 261
#define VOID 262
#define IF 263
#define ELSE 264
#define RETURN 265
#define IMPORT 266
#define TRUE 267
#define FALSE 268
#define WHILE 269
#define FFC 270
#define ITEM 271
#define ITEMCLASS 272
#define GLOBAL 273
#define ASSIGN 274
#define SEMICOLON 275
#define COMMA 276
#define LBRACKET 277
#define RBRACKET 278
#define LPAREN 279
#define RPAREN 280
#define DOT 281
#define LBRACE 282
#define RBRACE 283
#define ARROW 284
#define NUMBER 285
#define IDENTIFIER 286
#define QUOTEDSTRING 287
#define RSHIFT 288
#define LSHIFT 289
#define BITXOR 290
#define BITOR 291
#define BITAND 292
#define OR 293
#define AND 294
#define BITNOT 295
#define NOT 296
#define DECREMENT 297
#define INCREMENT 298
#define NE 299
#define EQ 300
#define GT 301
#define GE 302
#define LT 303
#define LE 304
#define MINUS 305
#define PLUS 306
#define DIVIDE 307
#define TIMES 308
#define MODULO 309




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

#if ! defined (YYLTYPE) && ! defined (YYLTYPE_IS_DECLARED)
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE yylloc;


