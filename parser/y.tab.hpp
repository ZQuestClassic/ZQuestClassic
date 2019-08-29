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
     BREAK = 274,
     CONTINUE = 275,
     CONST = 276,
     DO = 277,
     ASSIGN = 278,
     SEMICOLON = 279,
     COMMA = 280,
     LBRACKET = 281,
     RBRACKET = 282,
     LPAREN = 283,
     RPAREN = 284,
     DOT = 285,
     LBRACE = 286,
     RBRACE = 287,
     ARROW = 288,
     NUMBER = 289,
     PLUSASSIGN = 290,
     MINUSASSIGN = 291,
     TIMESASSIGN = 292,
     DIVIDEASSIGN = 293,
     ANDASSIGN = 294,
     ORASSIGN = 295,
     BITANDASSIGN = 296,
     BITORASSIGN = 297,
     BITXORASSIGN = 298,
     MODULOASSIGN = 299,
     LSHIFTASSIGN = 300,
     RSHIFTASSIGN = 301,
     IDENTIFIER = 302,
     QUOTEDSTRING = 303,
     RSHIFT = 304,
     LSHIFT = 305,
     BITXOR = 306,
     BITOR = 307,
     BITAND = 308,
     OR = 309,
     AND = 310,
     BITNOT = 311,
     NOT = 312,
     DECREMENT = 313,
     INCREMENT = 314,
     NE = 315,
     EQ = 316,
     GT = 317,
     GE = 318,
     LT = 319,
     LE = 320,
     MINUS = 321,
     PLUS = 322,
     DIVIDE = 323,
     TIMES = 324,
     MODULO = 325
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
#define BREAK 274
#define CONTINUE 275
#define CONST 276
#define DO 277
#define ASSIGN 278
#define SEMICOLON 279
#define COMMA 280
#define LBRACKET 281
#define RBRACKET 282
#define LPAREN 283
#define RPAREN 284
#define DOT 285
#define LBRACE 286
#define RBRACE 287
#define ARROW 288
#define NUMBER 289
#define PLUSASSIGN 290
#define MINUSASSIGN 291
#define TIMESASSIGN 292
#define DIVIDEASSIGN 293
#define ANDASSIGN 294
#define ORASSIGN 295
#define BITANDASSIGN 296
#define BITORASSIGN 297
#define BITXORASSIGN 298
#define MODULOASSIGN 299
#define LSHIFTASSIGN 300
#define RSHIFTASSIGN 301
#define IDENTIFIER 302
#define QUOTEDSTRING 303
#define RSHIFT 304
#define LSHIFT 305
#define BITXOR 306
#define BITOR 307
#define BITAND 308
#define OR 309
#define AND 310
#define BITNOT 311
#define NOT 312
#define DECREMENT 313
#define INCREMENT 314
#define NE 315
#define EQ 316
#define GT 317
#define GE 318
#define LT 319
#define LE 320
#define MINUS 321
#define PLUS 322
#define DIVIDE 323
#define TIMES 324
#define MODULO 325




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


