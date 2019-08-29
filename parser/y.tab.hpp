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
     ASSIGN = 277,
     SEMICOLON = 278,
     COMMA = 279,
     LBRACKET = 280,
     RBRACKET = 281,
     LPAREN = 282,
     RPAREN = 283,
     DOT = 284,
     LBRACE = 285,
     RBRACE = 286,
     ARROW = 287,
     NUMBER = 288,
     IDENTIFIER = 289,
     QUOTEDSTRING = 290,
     RSHIFT = 291,
     LSHIFT = 292,
     BITXOR = 293,
     BITOR = 294,
     BITAND = 295,
     OR = 296,
     AND = 297,
     BITNOT = 298,
     NOT = 299,
     DECREMENT = 300,
     INCREMENT = 301,
     NE = 302,
     EQ = 303,
     GT = 304,
     GE = 305,
     LT = 306,
     LE = 307,
     MINUS = 308,
     PLUS = 309,
     DIVIDE = 310,
     TIMES = 311,
     MODULO = 312
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
#define ASSIGN 277
#define SEMICOLON 278
#define COMMA 279
#define LBRACKET 280
#define RBRACKET 281
#define LPAREN 282
#define RPAREN 283
#define DOT 284
#define LBRACE 285
#define RBRACE 286
#define ARROW 287
#define NUMBER 288
#define IDENTIFIER 289
#define QUOTEDSTRING 290
#define RSHIFT 291
#define LSHIFT 292
#define BITXOR 293
#define BITOR 294
#define BITAND 295
#define OR 296
#define AND 297
#define BITNOT 298
#define NOT 299
#define DECREMENT 300
#define INCREMENT 301
#define NE 302
#define EQ 303
#define GT 304
#define GE 305
#define LT 306
#define LE 307
#define MINUS 308
#define PLUS 309
#define DIVIDE 310
#define TIMES 311
#define MODULO 312




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


