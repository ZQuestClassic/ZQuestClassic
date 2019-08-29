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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 1



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




/* Copy the first part of user declarations.  */
#line 10 "ffscript.ypp"

#include <stdlib.h>
#include <stdio.h>
#include "AST.h"
#include "UtilVisitors.h"
#include "../zsyssimple.h"
#define YYINCLUDED_STDLIB_H
extern int yydebug;
extern int yylineno;
extern char* yytext;
extern int yylex(void);
extern FILE *yyin;
extern AST *resAST;
extern void resetLexer();
void yyerror(const char* s);
string curfilename;
extern YYLTYPE noloc;

#define SHORTCUT(x,d1,d3,dd,l1,l2) ASTExpr *lval = (ASTExpr *)d1; \
					Clone c; \
					lval->execute(c,NULL); \
					x *rhs = new x(l2); \
					rhs->setFirstOperand((ASTExpr *)c.getResult()); \
					rhs->setSecondOperand((ASTExpr *)d3); \
					dd = new ASTStmtAssign(lval, rhs, l1);


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

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


/* Copy the second part of user declarations.  */


/* Line 219 of yacc.c.  */
#line 275 "y.tab.cpp"

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T) && (defined (__STDC__) || defined (__cplusplus))
# include <stddef.h> /* INFRINGES ON USER NAME SPACE */
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if defined (__STDC__) || defined (__cplusplus)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     define YYINCLUDED_STDLIB_H
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2005 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM ((YYSIZE_T) -1)
#  endif
#  ifdef __cplusplus
extern "C" {
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if (! defined (malloc) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if (! defined (free) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifdef __cplusplus
}
#  endif
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYLTYPE_IS_TRIVIAL) && YYLTYPE_IS_TRIVIAL \
             && defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   378

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  71
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  46
/* YYNRULES -- Number of rules. */
#define YYNRULES  135
/* YYNRULES -- Number of states. */
#define YYNSTATES  223

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   325

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     5,     8,     9,    11,    13,    15,    17,
      24,    29,    31,    33,    36,    40,    43,    46,    48,    51,
      56,    58,    61,    63,    65,    67,    69,    71,    73,    80,
      86,    90,    92,    96,    99,   102,   104,   107,   110,   113,
     115,   117,   119,   122,   124,   126,   128,   131,   134,   137,
     139,   141,   143,   145,   147,   149,   151,   153,   155,   156,
     158,   160,   162,   166,   170,   174,   178,   182,   186,   190,
     194,   198,   202,   206,   210,   214,   216,   218,   222,   224,
     228,   235,   239,   241,   245,   247,   251,   253,   257,   259,
     263,   265,   269,   271,   275,   277,   279,   281,   283,   285,
     287,   289,   291,   293,   297,   299,   301,   303,   307,   309,
     311,   313,   315,   318,   321,   324,   326,   330,   332,   335,
     338,   340,   342,   344,   346,   348,   353,   357,   361,   363,
     373,   379,   386,   392,   400,   403
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      72,     0,    -1,    73,    -1,    73,    74,    -1,    -1,    76,
      -1,    78,    -1,    84,    -1,    75,    -1,    21,     4,    47,
      23,    34,    24,    -1,    77,     3,    47,    79,    -1,    83,
      -1,    18,    -1,    11,    48,    -1,    31,    80,    32,    -1,
      31,    32,    -1,    81,    80,    -1,    81,    -1,    82,    24,
      -1,    82,    23,    94,    24,    -1,    84,    -1,    83,    47,
      -1,     4,    -1,     6,    -1,     7,    -1,    15,    -1,    16,
      -1,    17,    -1,    83,    47,    28,    85,    29,    86,    -1,
      83,    47,    28,    29,    86,    -1,    82,    25,    85,    -1,
      82,    -1,    31,    87,    32,    -1,    31,    32,    -1,    87,
      88,    -1,    88,    -1,    82,    24,    -1,    91,    24,    -1,
      90,    24,    -1,   112,    -1,   115,    -1,    86,    -1,   116,
      24,    -1,   113,    -1,   114,    -1,    24,    -1,    94,    24,
      -1,    19,    24,    -1,    20,    24,    -1,    82,    -1,    91,
      -1,    90,    -1,   112,    -1,   115,    -1,    86,    -1,   116,
      -1,   113,    -1,   114,    -1,    -1,    94,    -1,    19,    -1,
      20,    -1,    93,    35,    94,    -1,    93,    36,    94,    -1,
      93,    37,    94,    -1,    93,    38,    94,    -1,    93,    39,
      94,    -1,    93,    40,    94,    -1,    93,    41,    94,    -1,
      93,    42,    94,    -1,    93,    43,    94,    -1,    93,    45,
      94,    -1,    93,    46,    94,    -1,    93,    44,    94,    -1,
      92,    23,    94,    -1,    82,    -1,    93,    -1,    47,    30,
      47,    -1,    47,    -1,    93,    33,    47,    -1,    93,    33,
      47,    26,    94,    27,    -1,    94,    54,    95,    -1,    95,
      -1,    95,    55,    96,    -1,    96,    -1,    96,    52,    97,
      -1,    97,    -1,    97,    51,    98,    -1,    98,    -1,    98,
      53,    99,    -1,    99,    -1,    99,   102,   100,    -1,   100,
      -1,   100,   101,   103,    -1,   103,    -1,    50,    -1,    49,
      -1,    62,    -1,    63,    -1,    64,    -1,    65,    -1,    61,
      -1,    60,    -1,   103,   104,   105,    -1,   105,    -1,    67,
      -1,    66,    -1,   105,   106,   107,    -1,   107,    -1,    69,
      -1,    68,    -1,    70,    -1,    57,   107,    -1,    66,   107,
      -1,    56,   107,    -1,   108,    -1,    28,    94,    29,    -1,
      93,    -1,    93,    59,    -1,    93,    58,    -1,    34,    -1,
     109,    -1,   110,    -1,    12,    -1,    13,    -1,    93,    28,
     111,    29,    -1,    93,    28,    29,    -1,    94,    25,   111,
      -1,    94,    -1,     5,    28,    89,    24,    94,    24,    89,
      29,    88,    -1,    14,    28,    94,    29,    88,    -1,    22,
      88,    14,    28,    94,    29,    -1,     8,    28,    94,    29,
      88,    -1,     8,    28,    94,    29,    88,     9,    88,    -1,
      10,    94,    -1,    10,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   102,   102,   105,   108,   111,   112,   113,   114,   117,
     122,   129,   130,   133,   138,   139,   142,   145,   150,   151,
     156,   159,   165,   166,   167,   168,   169,   170,   173,   182,
     193,   196,   201,   202,   205,   209,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   260,   263,   264,   267,   271,   274,
     278,   286,   292,   295,   301,   304,   310,   313,   319,   322,
     328,   331,   337,   340,   346,   349,   350,   353,   354,   355,
     356,   357,   358,   361,   367,   370,   371,   374,   380,   383,
     384,   385,   388,   392,   396,   400,   403,   404,   405,   409,
     413,   415,   416,   419,   420,   423,   427,   433,   437,   443,
     450,   454,   458,   461,   467,   468
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SCRIPT", "FLOAT", "FOR", "BOOL", "VOID",
  "IF", "ELSE", "RETURN", "IMPORT", "TRUE", "FALSE", "WHILE", "FFC",
  "ITEM", "ITEMCLASS", "GLOBAL", "BREAK", "CONTINUE", "CONST", "DO",
  "ASSIGN", "SEMICOLON", "COMMA", "LBRACKET", "RBRACKET", "LPAREN",
  "RPAREN", "DOT", "LBRACE", "RBRACE", "ARROW", "NUMBER", "PLUSASSIGN",
  "MINUSASSIGN", "TIMESASSIGN", "DIVIDEASSIGN", "ANDASSIGN", "ORASSIGN",
  "BITANDASSIGN", "BITORASSIGN", "BITXORASSIGN", "MODULOASSIGN",
  "LSHIFTASSIGN", "RSHIFTASSIGN", "IDENTIFIER", "QUOTEDSTRING", "RSHIFT",
  "LSHIFT", "BITXOR", "BITOR", "BITAND", "OR", "AND", "BITNOT", "NOT",
  "DECREMENT", "INCREMENT", "NE", "EQ", "GT", "GE", "LT", "LE", "MINUS",
  "PLUS", "DIVIDE", "TIMES", "MODULO", "$accept", "Init", "Prog",
  "GlobalStmt", "ConstDecl", "Script", "ScriptType", "Import",
  "ScriptBlock", "ScriptStmtList", "ScriptStmt", "VarDecl", "Type",
  "FuncDecl", "ParamList", "Block", "StmtList", "Stmt", "StmtNoSemi",
  "ShortcutAssignStmt", "AssignStmt", "LVal", "DotExpr", "Expr", "Expr15",
  "Expr16", "Expr17", "Expr18", "Expr2", "Expr25", "ShiftOp", "RelOp",
  "Expr3", "AddOp", "Expr4", "MultOp", "Expr5", "Factor", "BoolConstant",
  "FuncCall", "ExprList", "ForStmt", "WhileStmt", "DoStmt", "IfStmt",
  "ReturnStmt", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    71,    72,    73,    73,    74,    74,    74,    74,    75,
      76,    77,    77,    78,    79,    79,    80,    80,    81,    81,
      81,    82,    83,    83,    83,    83,    83,    83,    84,    84,
      85,    85,    86,    86,    87,    87,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    91,    92,    92,    93,    93,    93,
      93,    94,    94,    95,    95,    96,    96,    97,    97,    98,
      98,    99,    99,   100,   100,   101,   101,   102,   102,   102,
     102,   102,   102,   103,   103,   104,   104,   105,   105,   106,
     106,   106,   107,   107,   107,   107,   108,   108,   108,   108,
     108,   108,   108,   109,   109,   110,   110,   111,   111,   112,
     113,   114,   115,   115,   116,   116
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     2,     0,     1,     1,     1,     1,     6,
       4,     1,     1,     2,     3,     2,     2,     1,     2,     4,
       1,     2,     1,     1,     1,     1,     1,     1,     6,     5,
       3,     1,     3,     2,     2,     1,     2,     2,     2,     1,
       1,     1,     2,     1,     1,     1,     2,     2,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     3,     1,     3,
       6,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     3,     1,     1,
       1,     1,     2,     2,     2,     1,     3,     1,     2,     2,
       1,     1,     1,     1,     1,     4,     3,     3,     1,     9,
       5,     6,     5,     7,     2,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       4,     0,     2,     1,    22,    23,    24,     0,    25,    26,
      27,    12,     0,     3,     8,     5,     0,     6,    11,     7,
      13,     0,     0,     0,     0,     0,     0,     0,     0,    10,
       0,    31,     0,     0,     0,    15,     0,    17,     0,     0,
      20,     0,    29,     0,    21,     0,     9,    14,    16,     0,
      18,    21,     0,     0,   135,   123,   124,     0,     0,     0,
       0,    45,     0,    33,   120,    78,     0,     0,     0,    75,
      41,     0,    35,     0,     0,     0,   117,     0,    82,    84,
      86,    88,    90,    92,    94,   104,   108,   115,   121,   122,
      39,    43,    44,    40,     0,    30,    28,   117,     0,    58,
       0,   134,     0,    47,    48,     0,     0,     0,   114,   112,
     113,    36,    32,    34,    38,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   119,   118,    46,     0,     0,     0,     0,     0,   102,
     101,    97,    98,    99,   100,     0,    96,    95,     0,   106,
     105,     0,   110,   109,   111,     0,    42,    19,    60,    61,
      49,    54,     0,    51,    50,    59,    52,    56,    57,    53,
      55,     0,     0,     0,   116,    77,    74,   126,   128,     0,
      79,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      73,    71,    72,    81,    83,    85,    87,    89,    91,    93,
     103,   107,     0,     0,     0,     0,     0,   125,     0,     0,
     132,   130,     0,   127,     0,    58,     0,   131,    80,     0,
     133,     0,   129
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,     1,     2,    13,    14,    15,    16,    17,    29,    36,
      37,    69,    32,    40,    33,    70,    71,    72,   162,    73,
      74,    75,    97,    77,    78,    79,    80,    81,    82,    83,
     148,   145,    84,   151,    85,   155,    86,    87,    88,    89,
     179,    90,    91,    92,    93,    94
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -96
static const short int yypact[] =
{
     -96,    26,   221,   -96,   -96,   -96,   -96,   -17,   -96,   -96,
     -96,   -96,    31,   -96,   -96,   -96,    37,   -96,    -5,   -96,
     -96,    36,    50,    16,    54,    65,    98,    67,   102,   -96,
      76,    91,    89,   114,   122,   -96,   116,   266,    -3,   105,
     -96,   125,   -96,   266,   -96,    76,   -96,   -96,   -96,   312,
     -96,    16,   126,   127,   312,   -96,   -96,   130,   136,   137,
     243,   -96,   312,   -96,   -96,   132,   312,   312,   312,   140,
     -96,   188,   -96,   145,   150,   152,    13,   -11,   121,   133,
     128,   131,   224,    30,    44,   -40,   -96,   -96,   -96,   -96,
     -96,   -96,   -96,   -96,   163,   -96,   -96,     4,    -9,   298,
     312,   134,   312,   -96,   -96,   176,    45,   159,   -96,   -96,
     -96,   -96,   -96,   -96,   -96,   -96,   312,   309,   162,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   -96,   -96,   -96,   312,   312,   312,   312,   312,   -96,
     -96,   -96,   -96,   -96,   -96,   312,   -96,   -96,   312,   -96,
     -96,   312,   -96,   -96,   -96,   312,   -96,   -96,   -96,   -96,
     174,   -96,   175,   -96,   -96,   134,   -96,   -96,   -96,   -96,
     -96,    46,    49,   183,   -96,   -96,   134,   -96,    22,   184,
     189,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   121,   133,   128,   131,   224,    30,    44,
     -40,   -96,   312,   243,   243,   312,   312,   -96,   312,    14,
     205,   -96,    69,   -96,    12,   298,   243,   -96,   -96,   192,
     -96,   243,   -96
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
     -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,   180,
     -96,   -26,    32,   216,   181,   -29,   -96,   -53,     8,   -95,
     -94,   -96,   -38,   -35,    92,    94,    95,    93,    96,    88,
     -96,   -96,   104,   -96,    90,   -96,   -43,   -96,   -96,   -96,
      34,   -93,   -91,   -90,   -89,   -87
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -77
static const short int yytable[] =
{
      31,    42,    38,    76,   163,   164,   166,   105,   167,   168,
     169,    38,   170,   133,    98,   157,    96,    31,   113,   101,
      49,    50,    76,   108,   109,   110,     3,   106,   152,   153,
     154,    20,   117,    76,    18,    21,   -76,   118,   215,   218,
      22,   117,    23,   134,    26,   134,   118,   206,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
      39,    76,   131,   132,   165,   171,   134,   172,   134,    39,
     161,   131,   132,   160,   174,   203,   134,    27,   204,   146,
     147,   176,   178,    24,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    28,    25,   217,   134,
     134,    34,     4,   134,     5,     6,     4,    41,     5,     6,
     149,   150,   201,     8,     9,    10,    43,     8,     9,    10,
     163,   164,   166,   134,   167,   168,   169,    30,   170,     4,
      52,     5,     6,    53,    35,    54,    44,    55,    56,    57,
       8,     9,    10,    45,    58,    59,    46,    60,    47,    61,
     210,   211,    51,    62,    99,   100,    41,    63,   102,    64,
     103,   104,   107,   220,   111,    76,    76,   209,   222,   114,
     212,   178,    65,   214,   115,   116,   135,    76,    76,   137,
     165,    66,    67,    76,   138,   136,   161,   156,   134,   160,
     173,    68,     4,    52,     5,     6,    53,   -75,    54,   202,
      55,    56,    57,     8,     9,    10,   175,    58,    59,   180,
      60,   205,    61,   207,   216,   208,    62,    48,    19,    41,
     112,   221,    64,   219,    95,     4,   193,     5,     6,   194,
     196,   195,     7,   198,   197,    65,     8,     9,    10,    11,
     213,   200,    12,     0,    66,    67,     0,     4,    52,     5,
       6,    53,   199,    54,    68,    55,    56,    57,     8,     9,
      10,     0,    58,    59,     0,    60,     0,    61,     0,     0,
       4,    62,     5,     6,    41,     0,     0,    64,     0,     0,
       0,     8,     9,    10,   139,   140,   141,   142,   143,   144,
      65,     0,     0,     0,     0,     0,     0,     0,     0,    66,
      67,     0,     4,    52,     5,     6,    53,     0,    54,    68,
      55,    56,    57,     8,     9,    10,     0,   158,   159,     0,
      60,    55,    56,     0,    55,    56,    62,     0,     0,    41,
       0,     0,    64,     0,     0,     0,     0,    62,   177,     0,
      62,     0,     0,    64,     0,    65,    64,     0,     0,     0,
       0,     0,     0,     0,    66,    67,    65,     0,     0,    65,
       0,     0,     0,     0,    68,    66,    67,     0,    66,    67,
       0,     0,     0,     0,     0,    68,     0,     0,    68
};

static const short int yycheck[] =
{
      26,    30,    28,    41,    99,    99,    99,    60,    99,    99,
      99,    37,    99,    24,    49,    24,    45,    43,    71,    54,
      23,    24,    60,    66,    67,    68,     0,    62,    68,    69,
      70,    48,    28,    71,     2,     4,    23,    33,    24,    27,
       3,    28,    47,    54,    28,    54,    33,    25,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      28,    99,    58,    59,    99,   100,    54,   102,    54,    37,
      99,    58,    59,    99,    29,    29,    54,    23,    29,    49,
      50,   116,   117,    47,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,    31,    47,    29,    54,
      54,    34,     4,    54,     6,     7,     4,    31,     6,     7,
      66,    67,   155,    15,    16,    17,    25,    15,    16,    17,
     215,   215,   215,    54,   215,   215,   215,    29,   215,     4,
       5,     6,     7,     8,    32,    10,    47,    12,    13,    14,
      15,    16,    17,    29,    19,    20,    24,    22,    32,    24,
     203,   204,    47,    28,    28,    28,    31,    32,    28,    34,
      24,    24,    30,   216,    24,   203,   204,   202,   221,    24,
     205,   206,    47,   208,    24,    23,    55,   215,   216,    51,
     215,    56,    57,   221,    53,    52,   215,    24,    54,   215,
      14,    66,     4,     5,     6,     7,     8,    23,    10,    24,
      12,    13,    14,    15,    16,    17,    47,    19,    20,    47,
      22,    28,    24,    29,     9,    26,    28,    37,     2,    31,
      32,    29,    34,   215,    43,     4,   134,     6,     7,   135,
     137,   136,    11,   145,   138,    47,    15,    16,    17,    18,
     206,   151,    21,    -1,    56,    57,    -1,     4,     5,     6,
       7,     8,   148,    10,    66,    12,    13,    14,    15,    16,
      17,    -1,    19,    20,    -1,    22,    -1,    24,    -1,    -1,
       4,    28,     6,     7,    31,    -1,    -1,    34,    -1,    -1,
      -1,    15,    16,    17,    60,    61,    62,    63,    64,    65,
      47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      57,    -1,     4,     5,     6,     7,     8,    -1,    10,    66,
      12,    13,    14,    15,    16,    17,    -1,    19,    20,    -1,
      22,    12,    13,    -1,    12,    13,    28,    -1,    -1,    31,
      -1,    -1,    34,    -1,    -1,    -1,    -1,    28,    29,    -1,
      28,    -1,    -1,    34,    -1,    47,    34,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    57,    47,    -1,    -1,    47,
      -1,    -1,    -1,    -1,    66,    56,    57,    -1,    56,    57,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    66
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    72,    73,     0,     4,     6,     7,    11,    15,    16,
      17,    18,    21,    74,    75,    76,    77,    78,    83,    84,
      48,     4,     3,    47,    47,    47,    28,    23,    31,    79,
      29,    82,    83,    85,    34,    32,    80,    81,    82,    83,
      84,    31,    86,    25,    47,    29,    24,    32,    80,    23,
      24,    47,     5,     8,    10,    12,    13,    14,    19,    20,
      22,    24,    28,    32,    34,    47,    56,    57,    66,    82,
      86,    87,    88,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   103,   105,   107,   108,   109,   110,
     112,   113,   114,   115,   116,    85,    86,    93,    94,    28,
      28,    94,    28,    24,    24,    88,    94,    30,   107,   107,
     107,    24,    32,    88,    24,    24,    23,    28,    33,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    58,    59,    24,    54,    55,    52,    51,    53,    60,
      61,    62,    63,    64,    65,   102,    49,    50,   101,    66,
      67,   104,    68,    69,    70,   106,    24,    24,    19,    20,
      82,    86,    89,    90,    91,    94,   112,   113,   114,   115,
     116,    94,    94,    14,    29,    47,    94,    29,    94,   111,
      47,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    95,    96,    97,    98,    99,   100,   103,
     105,   107,    24,    29,    29,    28,    25,    29,    26,    94,
      88,    88,    94,   111,    94,    24,     9,    29,    27,    89,
      88,    29,    88
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (0)
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
              (Loc).first_line, (Loc).first_column,	\
              (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr,					\
                  Type, Value, Location);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname[yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      size_t yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

#endif /* YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;
  (void) yylocationp;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");

# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;
  (void) yylocationp;

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;
/* Location data for the look-ahead symbol.  */
YYLTYPE yylloc;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()
    ;
#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended. */
  YYLTYPE yyerror_range[2];

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;
#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 0;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
	YYSTACK_RELOCATE (yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
  *++yylsp = yylloc;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, yylsp - yylen, yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 102 "ffscript.ypp"
    {resAST = new ASTProgram((ASTDeclList *)(yyvsp[0]), (yylsp[0]));;}
    break;

  case 3:
#line 105 "ffscript.ypp"
    {ASTDeclList *list = (ASTDeclList *)(yyvsp[-1]);
				list->addDeclaration((ASTDecl *)(yyvsp[0]));
				(yyval) = list;;}
    break;

  case 4:
#line 108 "ffscript.ypp"
    {(yyval) = new ASTDeclList(noloc);;}
    break;

  case 5:
#line 111 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 6:
#line 112 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 7:
#line 113 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 8:
#line 114 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 9:
#line 117 "ffscript.ypp"
    {ASTString *name = (ASTString *)(yyvsp[-3]);
															ASTFloat *val = (ASTFloat *)(yyvsp[-1]);
															(yyval) = new ASTConstDecl(name->getValue(), val,(yylsp[-5]));
															delete name;;}
    break;

  case 10:
#line 122 "ffscript.ypp"
    {ASTType *type = (ASTType *)(yyvsp[-3]);
								   ASTString *name = (ASTString *)(yyvsp[-1]);
								   ASTDeclList *sblock = (ASTDeclList *)(yyvsp[0]);
								   (yyval) = new ASTScript(type, name->getValue(), sblock,(yylsp[-3]));
								   delete name;;}
    break;

  case 11:
#line 129 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 12:
#line 130 "ffscript.ypp"
    {(yyval) = new ASTTypeGlobal((yylsp[0]));;}
    break;

  case 13:
#line 133 "ffscript.ypp"
    {ASTString *str = (ASTString *)(yyvsp[0]);
					 (yyval) = new ASTImportDecl(str->getValue(),(yylsp[-1]));
					 delete str;;}
    break;

  case 14:
#line 138 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 15:
#line 139 "ffscript.ypp"
    {(yyval) = new ASTDeclList((yylsp[-1]));;}
    break;

  case 16:
#line 142 "ffscript.ypp"
    {ASTDeclList *dl = (ASTDeclList *)(yyvsp[0]);
							  dl->addDeclaration((ASTDecl *)(yyvsp[-1]));
							  (yyval) = dl;;}
    break;

  case 17:
#line 145 "ffscript.ypp"
    {ASTDeclList *dl = new ASTDeclList((yylsp[0]));
			  dl->addDeclaration((ASTDecl *)(yyvsp[0]));
			  (yyval) = dl;;}
    break;

  case 18:
#line 150 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 19:
#line 151 "ffscript.ypp"
    {ASTVarDecl *vd = (ASTVarDecl *)(yyvsp[-3]);
						   Clone c;
						   vd->getType()->execute(c,NULL);
						   (yyval) = new ASTVarDeclInitializer((ASTType *)c.getResult(), vd->getName(), (ASTExpr *)(yyvsp[-1]),(yylsp[-3]));
						   delete vd;;}
    break;

  case 20:
#line 156 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 21:
#line 159 "ffscript.ypp"
    {ASTType *type = (ASTType *)(yyvsp[-1]);
				   ASTString *name = (ASTString *)(yyvsp[0]);
				   (yyval) = new ASTVarDecl(type, name->getValue(),(yylsp[-1]));
				   delete name;;}
    break;

  case 22:
#line 165 "ffscript.ypp"
    {(yyval) = new ASTTypeFloat((yylsp[0]));;}
    break;

  case 23:
#line 166 "ffscript.ypp"
    {(yyval) = new ASTTypeBool((yylsp[0]));;}
    break;

  case 24:
#line 167 "ffscript.ypp"
    {(yyval) = new ASTTypeVoid((yylsp[0]));;}
    break;

  case 25:
#line 168 "ffscript.ypp"
    {(yyval) = new ASTTypeFFC((yylsp[0]));;}
    break;

  case 26:
#line 169 "ffscript.ypp"
    {(yyval) = new ASTTypeItem((yylsp[0]));;}
    break;

  case 27:
#line 170 "ffscript.ypp"
    {(yyval) = new ASTTypeItemclass((yylsp[0]));;}
    break;

  case 28:
#line 173 "ffscript.ypp"
    {ASTFuncDecl *fd = (ASTFuncDecl *)(yyvsp[-2]);
									    ASTType *rettype = (ASTType *)(yyvsp[-5]);
									    ASTString *name = (ASTString *)(yyvsp[-4]);
									    ASTBlock *block = (ASTBlock *)(yyvsp[0]);
									    fd->setName(name->getValue());
									    delete name;
									    fd->setReturnType(rettype);
									    fd->setBlock(block);
									    (yyval)=fd;;}
    break;

  case 29:
#line 182 "ffscript.ypp"
    {ASTFuncDecl *fd = new ASTFuncDecl((yylsp[-4]));
							   ASTType *rettype = (ASTType *)(yyvsp[-4]);
							   ASTString *name = (ASTString *)(yyvsp[-3]);
							   ASTBlock *block = (ASTBlock *)(yyvsp[0]);
							   fd->setName(name->getValue());
							   delete name;
							   fd->setReturnType(rettype);
							   fd->setBlock(block);
							   (yyval)=fd;;}
    break;

  case 30:
#line 193 "ffscript.ypp"
    {ASTFuncDecl *fd = (ASTFuncDecl *)(yyvsp[0]);
						 fd->addParam((ASTVarDecl *)(yyvsp[-2]));
						 (yyval)=fd;;}
    break;

  case 31:
#line 196 "ffscript.ypp"
    {ASTFuncDecl *fd = new ASTFuncDecl((yylsp[0]));
		     fd->addParam((ASTVarDecl *)(yyvsp[0]));
		     (yyval)=fd;;}
    break;

  case 32:
#line 201 "ffscript.ypp"
    {(yyval)=(yyvsp[-1]);;}
    break;

  case 33:
#line 202 "ffscript.ypp"
    {(yyval) = new ASTBlock((yylsp[-1]));;}
    break;

  case 34:
#line 205 "ffscript.ypp"
    {ASTBlock *block = (ASTBlock *)(yyvsp[-1]);
				  ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
				  block->addStatement(stmt);
				  (yyval) = block;;}
    break;

  case 35:
#line 209 "ffscript.ypp"
    {ASTBlock *block = new ASTBlock((yylsp[0]));
		  ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
		  block->addStatement(stmt);
		  (yyval) = block;;}
    break;

  case 36:
#line 215 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 37:
#line 216 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 38:
#line 217 "ffscript.ypp"
    {(yyval)=(yyvsp[-1]);;}
    break;

  case 39:
#line 218 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 40:
#line 219 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 41:
#line 220 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 42:
#line 221 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 43:
#line 222 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 44:
#line 223 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 45:
#line 224 "ffscript.ypp"
    {(yyval) = new ASTStmtEmpty((yylsp[0]));;}
    break;

  case 46:
#line 225 "ffscript.ypp"
    {(yyval)=(yyvsp[-1]);;}
    break;

  case 47:
#line 226 "ffscript.ypp"
    {(yyval) = new ASTStmtBreak((yylsp[-1]));;}
    break;

  case 48:
#line 227 "ffscript.ypp"
    {(yyval) = new ASTStmtContinue((yylsp[-1]));;}
    break;

  case 49:
#line 230 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 50:
#line 231 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 51:
#line 232 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 52:
#line 233 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 53:
#line 234 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 54:
#line 235 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 55:
#line 236 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 56:
#line 237 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 57:
#line 238 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 58:
#line 239 "ffscript.ypp"
    {(yyval) = new ASTStmtEmpty(noloc);;}
    break;

  case 59:
#line 240 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 60:
#line 241 "ffscript.ypp"
    {(yyval) = new ASTStmtBreak((yylsp[0]));;}
    break;

  case 61:
#line 242 "ffscript.ypp"
    {(yyval) = new ASTStmtContinue((yylsp[0]));;}
    break;

  case 62:
#line 245 "ffscript.ypp"
    {SHORTCUT(ASTExprPlus,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 63:
#line 246 "ffscript.ypp"
    {SHORTCUT(ASTExprMinus,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 64:
#line 247 "ffscript.ypp"
    {SHORTCUT(ASTExprTimes,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 65:
#line 248 "ffscript.ypp"
    {SHORTCUT(ASTExprDivide,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 66:
#line 249 "ffscript.ypp"
    {SHORTCUT(ASTExprAnd,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 67:
#line 250 "ffscript.ypp"
    {SHORTCUT(ASTExprOr,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 68:
#line 251 "ffscript.ypp"
    {SHORTCUT(ASTExprBitAnd,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 69:
#line 252 "ffscript.ypp"
    {SHORTCUT(ASTExprBitOr,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 70:
#line 253 "ffscript.ypp"
    {SHORTCUT(ASTExprBitXor,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 71:
#line 254 "ffscript.ypp"
    {SHORTCUT(ASTExprLShift,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 72:
#line 255 "ffscript.ypp"
    {SHORTCUT(ASTExprRShift,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 73:
#line 256 "ffscript.ypp"
    {SHORTCUT(ASTExprModulo,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 74:
#line 260 "ffscript.ypp"
    {(yyval) = new ASTStmtAssign((ASTStmt *)(yyvsp[-2]), (ASTExpr *)(yyvsp[0]),(yylsp[-2]));;}
    break;

  case 75:
#line 263 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 76:
#line 264 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 77:
#line 267 "ffscript.ypp"
    {ASTString *lval = (ASTString *)(yyvsp[-2]);
						 ASTString *rval = (ASTString *)(yyvsp[0]);
						 (yyval) = new ASTExprDot(lval->getValue(), rval->getValue(),(yylsp[-2]));
						 delete lval; delete rval;;}
    break;

  case 78:
#line 271 "ffscript.ypp"
    {ASTString *rval = (ASTString *)(yyvsp[0]);
				  (yyval) = new ASTExprDot("", rval->getValue(),(yylsp[0]));
				  delete rval;;}
    break;

  case 79:
#line 274 "ffscript.ypp"
    {ASTExpr *id = (ASTExpr *)(yyvsp[-2]);
								ASTString *rval = (ASTString *)(yyvsp[0]);
								(yyval) = new ASTExprArrow(id, rval->getValue(), (yylsp[-2]));
								delete rval;;}
    break;

  case 80:
#line 278 "ffscript.ypp"
    {ASTExpr *id = (ASTExpr *)(yyvsp[-5]);
											       	   ASTString *rval = (ASTString *)(yyvsp[-3]);
													   ASTExpr *num = (ASTExpr *)(yyvsp[-1]);
													   ASTExprArrow *res = new ASTExprArrow(id, rval->getValue(), (yylsp[-5]));
													   res->setIndex(num);
													   (yyval) = res;;}
    break;

  case 81:
#line 286 "ffscript.ypp"
    {ASTLogExpr *e = new ASTExprOr((yylsp[-1]));
				 ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				 ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				 e->setFirstOperand(left);
				 e->setSecondOperand(right);
				 (yyval)=e;;}
    break;

  case 82:
#line 292 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 83:
#line 295 "ffscript.ypp"
    {ASTLogExpr *e = new ASTExprAnd((yylsp[-1]));
				 ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				 ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				 e->setFirstOperand(left);
				 e->setSecondOperand(right);
				 (yyval)=e;;}
    break;

  case 84:
#line 301 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 85:
#line 304 "ffscript.ypp"
    {ASTBitExpr *e = new ASTExprBitOr((yylsp[-1]));
							  ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							  ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							  e->setFirstOperand(left);
							  e->setSecondOperand(right);
							  (yyval)=e;;}
    break;

  case 86:
#line 310 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 87:
#line 313 "ffscript.ypp"
    {ASTBitExpr *e = new ASTExprBitXor((yylsp[-1]));
							   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							   e->setFirstOperand(left);
							   e->setSecondOperand(right);
							   (yyval)=e;;}
    break;

  case 88:
#line 319 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 89:
#line 322 "ffscript.ypp"
    {ASTBitExpr *e = new ASTExprBitAnd((yylsp[-1]));
							   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							   e->setFirstOperand(left);
							   e->setSecondOperand(right);
							   (yyval)=e;;}
    break;

  case 90:
#line 328 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 91:
#line 331 "ffscript.ypp"
    {ASTRelExpr *e = (ASTRelExpr *)(yyvsp[-1]);
				   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				   e->setFirstOperand(left);
				   e->setSecondOperand(right);
				   (yyval)=e;;}
    break;

  case 92:
#line 337 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 93:
#line 340 "ffscript.ypp"
    {ASTShiftExpr *e = (ASTShiftExpr *)(yyvsp[-1]);
							   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							   e->setFirstOperand(left);
							   e->setSecondOperand(right);
							   (yyval)=e;;}
    break;

  case 94:
#line 346 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 95:
#line 349 "ffscript.ypp"
    {(yyval) = new ASTExprLShift((yylsp[0]));;}
    break;

  case 96:
#line 350 "ffscript.ypp"
    {(yyval) = new ASTExprRShift((yylsp[0]));;}
    break;

  case 97:
#line 353 "ffscript.ypp"
    {(yyval) = new ASTExprGT((yylsp[0]));;}
    break;

  case 98:
#line 354 "ffscript.ypp"
    {(yyval) = new ASTExprGE((yylsp[0]));;}
    break;

  case 99:
#line 355 "ffscript.ypp"
    {(yyval) = new ASTExprLT((yylsp[0]));;}
    break;

  case 100:
#line 356 "ffscript.ypp"
    {(yyval) = new ASTExprLE((yylsp[0]));;}
    break;

  case 101:
#line 357 "ffscript.ypp"
    {(yyval) = new ASTExprEQ((yylsp[0]));;}
    break;

  case 102:
#line 358 "ffscript.ypp"
    {(yyval) = new ASTExprNE((yylsp[0]));;}
    break;

  case 103:
#line 361 "ffscript.ypp"
    {ASTAddExpr *e = (ASTAddExpr *)(yyvsp[-1]);
				   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				   e->setFirstOperand(left);
				   e->setSecondOperand(right);
				   (yyval)=e;;}
    break;

  case 104:
#line 367 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 105:
#line 370 "ffscript.ypp"
    {(yyval) = new ASTExprPlus((yylsp[0]));;}
    break;

  case 106:
#line 371 "ffscript.ypp"
    {(yyval) = new ASTExprMinus((yylsp[0]));;}
    break;

  case 107:
#line 374 "ffscript.ypp"
    {ASTMultExpr *e = (ASTMultExpr *)(yyvsp[-1]);
				    ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				    ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				    e->setFirstOperand(left);
				    e->setSecondOperand(right);
				    (yyval)=e;;}
    break;

  case 108:
#line 380 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 109:
#line 383 "ffscript.ypp"
    {(yyval) = new ASTExprTimes((yylsp[0]));;}
    break;

  case 110:
#line 384 "ffscript.ypp"
    {(yyval) = new ASTExprDivide((yylsp[0]));;}
    break;

  case 111:
#line 385 "ffscript.ypp"
    {(yyval) = new ASTExprModulo((yylsp[0]));;}
    break;

  case 112:
#line 388 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprNot((yylsp[-1]));
			 ASTExpr *op = (ASTExpr *)(yyvsp[0]);
			 e->setOperand(op);
			 (yyval)=e;;}
    break;

  case 113:
#line 392 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprNegate((yylsp[-1]));
			   ASTExpr *op = (ASTExpr *)(yyvsp[0]);
			   e->setOperand(op);
			   (yyval)=e;;}
    break;

  case 114:
#line 396 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprBitNot((yylsp[-1]));
					ASTExpr *op = (ASTExpr *)(yyvsp[0]);
					e->setOperand(op);
					(yyval)=e;;}
    break;

  case 115:
#line 400 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 116:
#line 403 "ffscript.ypp"
    {(yyval)=(yyvsp[-1]);;}
    break;

  case 117:
#line 404 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 118:
#line 405 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprIncrement((yylsp[0]));
						 ASTExpr *op = (ASTExpr *)(yyvsp[-1]);
						 e->setOperand(op);
						 (yyval)=e;;}
    break;

  case 119:
#line 409 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprDecrement((yylsp[0]));
						 ASTExpr *op = (ASTExpr *)(yyvsp[-1]);
						 e->setOperand(op);
						 (yyval)=e;;}
    break;

  case 120:
#line 413 "ffscript.ypp"
    {ASTFloat *val = (ASTFloat *)(yyvsp[0]);
		    (yyval) = new ASTNumConstant(val,(yylsp[0]));;}
    break;

  case 121:
#line 415 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 122:
#line 416 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 123:
#line 419 "ffscript.ypp"
    {(yyval) = new ASTBoolConstant(true,(yylsp[0]));;}
    break;

  case 124:
#line 420 "ffscript.ypp"
    {(yyval) = new ASTBoolConstant(false,(yylsp[0]));;}
    break;

  case 125:
#line 423 "ffscript.ypp"
    {ASTFuncCall *fc = (ASTFuncCall *)(yyvsp[-1]);
							    ASTExpr *name = (ASTExpr *)(yyvsp[-3]);
							    fc->setName(name);
							    (yyval)=fc;;}
    break;

  case 126:
#line 427 "ffscript.ypp"
    {ASTFuncCall *fc = new ASTFuncCall((yylsp[-2]));
					    ASTExpr *name = (ASTExpr *)(yyvsp[-2]);
					    fc->setName(name);
					    (yyval)=fc;;}
    break;

  case 127:
#line 433 "ffscript.ypp"
    {ASTFuncCall *fc = (ASTFuncCall *)(yyvsp[0]);
					  ASTExpr *e = (ASTExpr *)(yyvsp[-2]);
					  fc->addParam(e);
					  (yyval) = fc;;}
    break;

  case 128:
#line 437 "ffscript.ypp"
    {ASTFuncCall *fc = new ASTFuncCall((yylsp[0]));
		  ASTExpr *e = (ASTExpr *)(yyvsp[0]);
		  fc->addParam(e);
		  (yyval) = fc;;}
    break;

  case 129:
#line 443 "ffscript.ypp"
    {ASTStmt *prec = (ASTStmt *)(yyvsp[-6]);
												ASTExpr *term = (ASTExpr *)(yyvsp[-4]);
												ASTStmt *incr = (ASTExpr *)(yyvsp[-2]);
												ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
												(yyval) = new ASTStmtFor(prec,term,incr,stmt,(yylsp[-8]));;}
    break;

  case 130:
#line 450 "ffscript.ypp"
    {ASTExpr *cond = (ASTExpr *)(yyvsp[-2]);
										   ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
										   (yyval) = new ASTStmtWhile(cond,stmt,(yylsp[-4]));;}
    break;

  case 131:
#line 454 "ffscript.ypp"
    {ASTExpr *cond = (ASTExpr *)(yyvsp[-1]);
										   ASTStmt *stmt = (ASTStmt *)(yyvsp[-4]);
										   (yyval) = new ASTStmtDo(cond,stmt,(yylsp[-5]));;}
    break;

  case 132:
#line 458 "ffscript.ypp"
    {ASTExpr *cond = (ASTExpr *)(yyvsp[-2]);
							  ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
							  (yyval) = new ASTStmtIf(cond,stmt,(yylsp[-4]));;}
    break;

  case 133:
#line 461 "ffscript.ypp"
    {ASTExpr *cond = (ASTExpr *)(yyvsp[-4]);
										ASTStmt *ifstmt = (ASTStmt *)(yyvsp[-2]);
										ASTStmt *elsestmt = (ASTStmt *)(yyvsp[0]);
										(yyval) = new ASTStmtIfElse(cond,ifstmt,elsestmt,(yylsp[-6]));;}
    break;

  case 134:
#line 467 "ffscript.ypp"
    {(yyval) = new ASTStmtReturnVal((ASTExpr *)(yyvsp[0]),(yylsp[-1]));;}
    break;

  case 135:
#line 468 "ffscript.ypp"
    {(yyval) = new ASTStmtReturn((yylsp[0]));;}
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 2358 "y.tab.cpp"

  yyvsp -= yylen;
  yyssp -= yylen;
  yylsp -= yylen;

  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  int yytype = YYTRANSLATE (yychar);
	  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
	  YYSIZE_T yysize = yysize0;
	  YYSIZE_T yysize1;
	  int yysize_overflow = 0;
	  char *yymsg = 0;
#	  define YYERROR_VERBOSE_ARGS_MAXIMUM 5
	  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
	  int yyx;

#if 0
	  /* This is so xgettext sees the translatable formats that are
	     constructed on the fly.  */
	  YY_("syntax error, unexpected %s");
	  YY_("syntax error, unexpected %s, expecting %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
#endif
	  char *yyfmt;
	  char const *yyf;
	  static char const yyunexpected[] = "syntax error, unexpected %s";
	  static char const yyexpecting[] = ", expecting %s";
	  static char const yyor[] = " or %s";
	  char yyformat[sizeof yyunexpected
			+ sizeof yyexpecting - 1
			+ ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
			   * (sizeof yyor - 1))];
	  char const *yyprefix = yyexpecting;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 1;

	  yyarg[0] = yytname[yytype];
	  yyfmt = yystpcpy (yyformat, yyunexpected);

	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
		  {
		    yycount = 1;
		    yysize = yysize0;
		    yyformat[sizeof yyunexpected - 1] = '\0';
		    break;
		  }
		yyarg[yycount++] = yytname[yyx];
		yysize1 = yysize + yytnamerr (0, yytname[yyx]);
		yysize_overflow |= yysize1 < yysize;
		yysize = yysize1;
		yyfmt = yystpcpy (yyfmt, yyprefix);
		yyprefix = yyor;
	      }

	  yyf = YY_(yyformat);
	  yysize1 = yysize + yystrlen (yyf);
	  yysize_overflow |= yysize1 < yysize;
	  yysize = yysize1;

	  if (!yysize_overflow && yysize <= YYSTACK_ALLOC_MAXIMUM)
	    yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg)
	    {
	      /* Avoid sprintf, as that infringes on the user's name space.
		 Don't have undefined behavior even if the translation
		 produced a string with the wrong number of "%s"s.  */
	      char *yyp = yymsg;
	      int yyi = 0;
	      while ((*yyp = *yyf))
		{
		  if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		    {
		      yyp += yytnamerr (yyp, yyarg[yyi++]);
		      yyf += 2;
		    }
		  else
		    {
		      yyp++;
		      yyf++;
		    }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    {
	      yyerror (YY_("syntax error"));
	      goto yyexhaustedlab;
	    }
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (YY_("syntax error"));
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  yylsp -= yylen;
  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping", yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though. */
  YYLLOC_DEFAULT (yyloc, yyerror_range - 1, 2);
  *++yylsp = yyloc;

  /* Shift the error token. */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK;
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 471 "ffscript.ypp"


/*        programs */
		
void yyerror(const char *s) {
	char temp[512];
    sprintf(temp, "line %d: %s, on token %s", yylineno, s, yytext);
    box_out(temp);
    box_eol();
}

int go(const char *f)
{
yyin = NULL;
resetLexer();
yyin = fopen(f, "r");
if(!yyin)
{
	box_out("Can't open input file");
	box_eol();
	return -1;
}
	curfilename = string(f);
	int result = yyparse();
	fclose(yyin);
	return result;
}

