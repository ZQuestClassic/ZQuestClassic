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
     NPC = 278,
     ASSIGN = 279,
     SEMICOLON = 280,
     COMMA = 281,
     LBRACKET = 282,
     RBRACKET = 283,
     LPAREN = 284,
     RPAREN = 285,
     DOT = 286,
     LBRACE = 287,
     RBRACE = 288,
     ARROW = 289,
     NUMBER = 290,
     PLUSASSIGN = 291,
     MINUSASSIGN = 292,
     TIMESASSIGN = 293,
     DIVIDEASSIGN = 294,
     ANDASSIGN = 295,
     ORASSIGN = 296,
     BITANDASSIGN = 297,
     BITORASSIGN = 298,
     BITXORASSIGN = 299,
     MODULOASSIGN = 300,
     LSHIFTASSIGN = 301,
     RSHIFTASSIGN = 302,
     IDENTIFIER = 303,
     QUOTEDSTRING = 304,
     RSHIFT = 305,
     LSHIFT = 306,
     BITXOR = 307,
     BITOR = 308,
     BITAND = 309,
     OR = 310,
     AND = 311,
     BITNOT = 312,
     NOT = 313,
     DECREMENT = 314,
     INCREMENT = 315,
     NE = 316,
     EQ = 317,
     GT = 318,
     GE = 319,
     LT = 320,
     LE = 321,
     MINUS = 322,
     PLUS = 323,
     DIVIDE = 324,
     TIMES = 325,
     MODULO = 326
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
#define NPC 278
#define ASSIGN 279
#define SEMICOLON 280
#define COMMA 281
#define LBRACKET 282
#define RBRACKET 283
#define LPAREN 284
#define RPAREN 285
#define DOT 286
#define LBRACE 287
#define RBRACE 288
#define ARROW 289
#define NUMBER 290
#define PLUSASSIGN 291
#define MINUSASSIGN 292
#define TIMESASSIGN 293
#define DIVIDEASSIGN 294
#define ANDASSIGN 295
#define ORASSIGN 296
#define BITANDASSIGN 297
#define BITORASSIGN 298
#define BITXORASSIGN 299
#define MODULOASSIGN 300
#define LSHIFTASSIGN 301
#define RSHIFTASSIGN 302
#define IDENTIFIER 303
#define QUOTEDSTRING 304
#define RSHIFT 305
#define LSHIFT 306
#define BITXOR 307
#define BITOR 308
#define BITAND 309
#define OR 310
#define AND 311
#define BITNOT 312
#define NOT 313
#define DECREMENT 314
#define INCREMENT 315
#define NE 316
#define EQ 317
#define GT 318
#define GE 319
#define LT 320
#define LE 321
#define MINUS 322
#define PLUS 323
#define DIVIDE 324
#define TIMES 325
#define MODULO 326




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


