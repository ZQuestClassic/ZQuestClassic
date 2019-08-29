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
     ASSIGN = 267,
     SEMICOLON = 268,
     COMMA = 269,
     LBRACKET = 270,
     RBRACKET = 271,
     LPAREN = 272,
     RPAREN = 273,
     DOT = 274,
     LBRACE = 275,
     RBRACE = 276,
     NUMBER = 277,
     IDENTIFIER = 278,
     QUOTEDSTRING = 279,
     OR = 280,
     AND = 281,
     NOT = 282,
     NE = 283,
     EQ = 284,
     GT = 285,
     GE = 286,
     LT = 287,
     LE = 288,
     MINUS = 289,
     PLUS = 290,
     DIVIDE = 291,
     TIMES = 292
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
#define ASSIGN 267
#define SEMICOLON 268
#define COMMA 269
#define LBRACKET 270
#define RBRACKET 271
#define LPAREN 272
#define RPAREN 273
#define DOT 274
#define LBRACE 275
#define RBRACE 276
#define NUMBER 277
#define IDENTIFIER 278
#define QUOTEDSTRING 279
#define OR 280
#define AND 281
#define NOT 282
#define NE 283
#define EQ 284
#define GT 285
#define GE 286
#define LT 287
#define LE 288
#define MINUS 289
#define PLUS 290
#define DIVIDE 291
#define TIMES 292




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



