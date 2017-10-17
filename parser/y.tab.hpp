
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


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
     LWEAPON = 279,
     EWEAPON = 280,
     ASSIGN = 281,
     SEMICOLON = 282,
     COMMA = 283,
     LBRACKET = 284,
     RBRACKET = 285,
     LPAREN = 286,
     RPAREN = 287,
     DOT = 288,
     LBRACE = 289,
     RBRACE = 290,
     ARROW = 291,
     NUMBER = 292,
     PLUSASSIGN = 293,
     MINUSASSIGN = 294,
     TIMESASSIGN = 295,
     DIVIDEASSIGN = 296,
     ANDASSIGN = 297,
     ORASSIGN = 298,
     BITANDASSIGN = 299,
     BITORASSIGN = 300,
     BITXORASSIGN = 301,
     MODULOASSIGN = 302,
     LSHIFTASSIGN = 303,
     RSHIFTASSIGN = 304,
     IDENTIFIER = 305,
     QUOTEDSTRING = 306,
     SINGLECHAR = 307,
     RSHIFT = 308,
     LSHIFT = 309,
     BITXOR = 310,
     BITOR = 311,
     BITAND = 312,
     OR = 313,
     AND = 314,
     BITNOT = 315,
     NOT = 316,
     DECREMENT = 317,
     INCREMENT = 318,
     NE = 319,
     EQ = 320,
     GT = 321,
     GE = 322,
     LT = 323,
     LE = 324,
     MINUS = 325,
     PLUS = 326,
     DIVIDE = 327,
     TIMES = 328,
     MODULO = 329
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
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

