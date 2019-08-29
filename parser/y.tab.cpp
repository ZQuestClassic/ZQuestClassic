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




/* Copy the first part of user declarations.  */
#line 10 "ffscript.ypp"

#include <stdlib.h>
#include <stdio.h>
#include "ScriptParser.h"
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
#line 234 "y.tab.cpp"

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
#define YYFINAL  18
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   248

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  55
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  43
/* YYNRULES -- Number of rules. */
#define YYNRULES  113
/* YYNRULES -- Number of states. */
#define YYNSTATES  176

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   309

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
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    11,    13,    15,    17,
      22,    24,    26,    29,    33,    36,    39,    41,    44,    49,
      51,    54,    56,    58,    60,    62,    64,    66,    73,    79,
      83,    85,    89,    92,    95,    97,   100,   103,   105,   107,
     109,   112,   114,   116,   119,   121,   123,   125,   127,   129,
     131,   133,   134,   136,   140,   142,   144,   148,   150,   154,
     161,   165,   167,   171,   173,   177,   179,   183,   185,   189,
     191,   195,   197,   201,   203,   205,   207,   209,   211,   213,
     215,   217,   219,   223,   225,   227,   229,   233,   235,   237,
     239,   241,   244,   247,   250,   252,   256,   258,   261,   264,
     266,   268,   270,   272,   274,   279,   283,   287,   289,   299,
     305,   311,   319,   322
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      56,     0,    -1,    57,    -1,    -1,    58,    57,    -1,    58,
      -1,    59,    -1,    61,    -1,    67,    -1,    60,     3,    31,
      62,    -1,    66,    -1,    18,    -1,    11,    32,    -1,    27,
      63,    28,    -1,    27,    28,    -1,    64,    63,    -1,    64,
      -1,    65,    20,    -1,    65,    19,    76,    20,    -1,    67,
      -1,    66,    31,    -1,     4,    -1,     6,    -1,     7,    -1,
      15,    -1,    16,    -1,    17,    -1,    66,    31,    24,    68,
      25,    69,    -1,    66,    31,    24,    25,    69,    -1,    65,
      21,    68,    -1,    65,    -1,    27,    70,    28,    -1,    27,
      28,    -1,    71,    70,    -1,    71,    -1,    65,    20,    -1,
      73,    20,    -1,    94,    -1,    96,    -1,    69,    -1,    97,
      20,    -1,    95,    -1,    20,    -1,    76,    20,    -1,    65,
      -1,    73,    -1,    94,    -1,    96,    -1,    69,    -1,    97,
      -1,    95,    -1,    -1,    76,    -1,    74,    19,    76,    -1,
      65,    -1,    75,    -1,    31,    26,    31,    -1,    31,    -1,
      75,    29,    31,    -1,    75,    29,    31,    22,    76,    23,
      -1,    76,    38,    77,    -1,    77,    -1,    77,    39,    78,
      -1,    78,    -1,    78,    36,    79,    -1,    79,    -1,    79,
      35,    80,    -1,    80,    -1,    80,    37,    81,    -1,    81,
      -1,    81,    84,    82,    -1,    82,    -1,    82,    83,    85,
      -1,    85,    -1,    34,    -1,    33,    -1,    46,    -1,    47,
      -1,    48,    -1,    49,    -1,    45,    -1,    44,    -1,    85,
      86,    87,    -1,    87,    -1,    51,    -1,    50,    -1,    87,
      88,    89,    -1,    89,    -1,    53,    -1,    52,    -1,    54,
      -1,    41,    89,    -1,    50,    89,    -1,    40,    89,    -1,
      90,    -1,    24,    76,    25,    -1,    75,    -1,    75,    43,
      -1,    75,    42,    -1,    30,    -1,    91,    -1,    92,    -1,
      12,    -1,    13,    -1,    75,    24,    93,    25,    -1,    75,
      24,    25,    -1,    76,    21,    93,    -1,    76,    -1,     5,
      24,    72,    20,    76,    20,    72,    25,    71,    -1,    14,
      24,    76,    25,    71,    -1,     8,    24,    76,    25,    71,
      -1,     8,    24,    76,    25,    71,     9,    71,    -1,    10,
      76,    -1,    10,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,    77,    77,    78,    81,    84,    89,    90,    91,    94,
     101,   102,   105,   110,   111,   114,   117,   122,   123,   128,
     131,   137,   138,   139,   140,   141,   142,   145,   154,   165,
     168,   173,   174,   177,   181,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   209,   212,   213,   216,   220,   223,   227,
     235,   241,   244,   250,   253,   259,   262,   268,   271,   277,
     280,   286,   289,   295,   298,   299,   302,   303,   304,   305,
     306,   307,   310,   316,   319,   320,   323,   329,   332,   333,
     334,   337,   341,   345,   349,   352,   353,   354,   358,   362,
     364,   365,   368,   369,   372,   376,   382,   386,   392,   399,
     403,   406,   412,   413
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SCRIPT", "FLOAT", "FOR", "BOOL", "VOID",
  "IF", "ELSE", "RETURN", "IMPORT", "TRUE", "FALSE", "WHILE", "FFC",
  "ITEM", "ITEMCLASS", "GLOBAL", "ASSIGN", "SEMICOLON", "COMMA",
  "LBRACKET", "RBRACKET", "LPAREN", "RPAREN", "DOT", "LBRACE", "RBRACE",
  "ARROW", "NUMBER", "IDENTIFIER", "QUOTEDSTRING", "RSHIFT", "LSHIFT",
  "BITXOR", "BITOR", "BITAND", "OR", "AND", "BITNOT", "NOT", "DECREMENT",
  "INCREMENT", "NE", "EQ", "GT", "GE", "LT", "LE", "MINUS", "PLUS",
  "DIVIDE", "TIMES", "MODULO", "$accept", "Init", "Prog", "GlobalStmt",
  "Script", "ScriptType", "Import", "ScriptBlock", "ScriptStmtList",
  "ScriptStmt", "VarDecl", "Type", "FuncDecl", "ParamList", "Block",
  "StmtList", "Stmt", "StmtNoSemi", "AssignStmt", "LVal", "DotExpr",
  "Expr", "Expr15", "Expr16", "Expr17", "Expr18", "Expr2", "Expr25",
  "ShiftOp", "RelOp", "Expr3", "AddOp", "Expr4", "MultOp", "Expr5",
  "Factor", "BoolConstant", "FuncCall", "ExprList", "ForStmt", "WhileStmt",
  "IfStmt", "ReturnStmt", 0
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
     305,   306,   307,   308,   309
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    55,    56,    56,    57,    57,    58,    58,    58,    59,
      60,    60,    61,    62,    62,    63,    63,    64,    64,    64,
      65,    66,    66,    66,    66,    66,    66,    67,    67,    68,
      68,    69,    69,    70,    70,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    73,    74,    74,    75,    75,    75,    75,
      76,    76,    77,    77,    78,    78,    79,    79,    80,    80,
      81,    81,    82,    82,    83,    83,    84,    84,    84,    84,
      84,    84,    85,    85,    86,    86,    87,    87,    88,    88,
      88,    89,    89,    89,    89,    90,    90,    90,    90,    90,
      90,    90,    91,    91,    92,    92,    93,    93,    94,    95,
      96,    96,    97,    97
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     0,     2,     1,     1,     1,     1,     4,
       1,     1,     2,     3,     2,     2,     1,     2,     4,     1,
       2,     1,     1,     1,     1,     1,     1,     6,     5,     3,
       1,     3,     2,     2,     1,     2,     2,     1,     1,     1,
       2,     1,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     0,     1,     3,     1,     1,     3,     1,     3,     6,
       3,     1,     3,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     1,     1,     3,     1,     1,     1,
       1,     2,     2,     2,     1,     3,     1,     2,     2,     1,
       1,     1,     1,     1,     4,     3,     3,     1,     9,     5,
       5,     7,     2,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       3,    21,    22,    23,     0,    24,    25,    26,    11,     0,
       2,     5,     6,     0,     7,    10,     8,    12,     1,     4,
       0,     0,     0,     0,     0,     9,     0,    30,     0,     0,
      14,     0,    16,     0,     0,    19,     0,    28,     0,    20,
       0,    13,    15,     0,    17,    20,     0,     0,   113,   102,
     103,     0,    42,     0,    32,    99,    57,     0,     0,     0,
      54,    39,     0,    34,     0,     0,    96,     0,    61,    63,
      65,    67,    69,    71,    73,    83,    87,    94,   100,   101,
      37,    41,    38,     0,    29,    27,    96,     0,    51,     0,
     112,     0,     0,     0,    93,    91,    92,    35,    31,    33,
      36,     0,     0,     0,    98,    97,    43,     0,     0,     0,
       0,     0,    81,    80,    76,    77,    78,    79,     0,    75,
      74,     0,    85,    84,     0,    89,    88,    90,     0,    40,
      18,    44,    48,     0,    45,    52,    46,    50,    47,    49,
       0,     0,    95,    56,    53,   105,   107,     0,    58,    60,
      62,    64,    66,    68,    70,    72,    82,    86,     0,     0,
       0,     0,   104,     0,     0,   110,   109,   106,     0,    51,
       0,    59,     0,   111,     0,   108
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,     9,    10,    11,    12,    13,    14,    25,    31,    32,
      60,    28,    16,    29,    61,    62,    63,   133,    64,    65,
      86,    67,    68,    69,    70,    71,    72,    73,   121,   118,
      74,   124,    75,   128,    76,    77,    78,    79,   147,    80,
      81,    82,    83
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -85
static const short int yypact[] =
{
     223,   -85,   -85,   -85,   -10,   -85,   -85,   -85,   -85,    50,
     -85,   223,   -85,    43,   -85,    72,   -85,   -85,   -85,   -85,
      74,    59,    85,   201,    62,   -85,    86,    98,    89,   104,
     -85,    95,   162,    15,    99,   -85,    94,   -85,   162,   -85,
      86,   -85,   -85,    12,   -85,    59,   108,   114,    12,   -85,
     -85,   116,   -85,    12,   -85,   -85,   117,    12,    12,    12,
     122,   -85,   124,   143,   134,   145,    51,    13,   126,   135,
     140,   144,   198,    58,    46,    63,   -85,   -85,   -85,   -85,
     -85,   -85,   -85,   160,   -85,   -85,    -3,    25,   182,    12,
     147,    12,    29,   151,   -85,   -85,   -85,   -85,   -85,   -85,
     -85,    12,     7,   169,   -85,   -85,   -85,    12,    12,    12,
      12,    12,   -85,   -85,   -85,   -85,   -85,   -85,    12,   -85,
     -85,    12,   -85,   -85,    12,   -85,   -85,   -85,    12,   -85,
     -85,   172,   -85,   181,   -85,   147,   -85,   -85,   -85,   -85,
      35,    57,   -85,   -85,   147,   -85,     6,   177,   188,   126,
     135,   140,   144,   198,    58,    46,    63,   -85,    12,   143,
     143,    12,   -85,    12,    36,   194,   -85,   -85,    -9,   182,
     143,   -85,   179,   -85,   143,   -85
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
     -85,   -85,   200,   -85,   -85,   -85,   -85,   -85,   183,   -85,
     -23,    17,    52,   176,   -24,   156,     2,    55,   -84,   -85,
     -33,   -30,   113,   120,   112,   115,   125,   119,   -85,   -85,
     110,   -85,   109,   -85,   -47,   -85,   -85,   -85,    87,   -83,
     -82,   -81,   -80
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -56
static const short int yytable[] =
{
      27,    33,    37,    66,   134,   136,   137,   138,   139,    33,
      94,    95,    96,    87,   171,    27,    85,    15,    90,    49,
      50,   102,    17,    92,    49,    50,   103,   161,    15,   107,
      66,    53,   145,   106,    43,    44,    53,    55,    56,   104,
     105,    34,    55,    56,   107,   130,    20,    57,    58,    34,
      18,   107,    57,    58,   142,    66,   169,    59,   135,   140,
     159,   141,    59,   107,   132,   131,     1,   107,     2,     3,
     -55,   144,   146,   107,   107,   102,    35,     5,     6,     7,
     103,   157,   160,    23,    35,   134,   136,   137,   138,   139,
      30,   119,   120,   104,   105,   107,   122,   123,     1,    46,
       2,     3,    47,    21,    48,    22,    49,    50,    51,     5,
       6,     7,    24,    36,    52,   125,   126,   127,    53,    38,
      39,    36,    54,    41,    55,    56,    66,    66,   164,    40,
      45,   146,    88,   168,    57,    58,    66,    66,    89,   135,
      91,    66,    97,    93,    59,   132,   131,     1,    46,     2,
       3,    47,    98,    48,   100,    49,    50,    51,     5,     6,
       7,   165,   166,    52,   101,   108,     1,    53,     2,     3,
      36,   109,   173,    55,    56,   110,   175,     5,     6,     7,
     129,   111,   143,    57,    58,   107,     1,    46,     2,     3,
      47,   -54,    48,    59,    49,    50,    51,     5,     6,     7,
     148,   158,   162,   170,   174,     1,    53,     2,     3,    36,
     163,    19,    55,    56,    84,    42,     5,     6,     7,    99,
     149,   151,    57,    58,   172,   152,    26,     1,   150,     2,
       3,   155,    59,   156,     4,     0,   153,   154,     5,     6,
       7,     8,   112,   113,   114,   115,   116,   117,   167
};

static const short int yycheck[] =
{
      23,    24,    26,    36,    88,    88,    88,    88,    88,    32,
      57,    58,    59,    43,    23,    38,    40,     0,    48,    12,
      13,    24,    32,    53,    12,    13,    29,    21,    11,    38,
      63,    24,    25,    20,    19,    20,    24,    30,    31,    42,
      43,    24,    30,    31,    38,    20,     3,    40,    41,    32,
       0,    38,    40,    41,    25,    88,    20,    50,    88,    89,
      25,    91,    50,    38,    88,    88,     4,    38,     6,     7,
      19,   101,   102,    38,    38,    24,    24,    15,    16,    17,
      29,   128,    25,    24,    32,   169,   169,   169,   169,   169,
      28,    33,    34,    42,    43,    38,    50,    51,     4,     5,
       6,     7,     8,    31,    10,    31,    12,    13,    14,    15,
      16,    17,    27,    27,    20,    52,    53,    54,    24,    21,
      31,    27,    28,    28,    30,    31,   159,   160,   158,    25,
      31,   161,    24,   163,    40,    41,   169,   170,    24,   169,
      24,   174,    20,    26,    50,   169,   169,     4,     5,     6,
       7,     8,    28,    10,    20,    12,    13,    14,    15,    16,
      17,   159,   160,    20,    19,    39,     4,    24,     6,     7,
      27,    36,   170,    30,    31,    35,   174,    15,    16,    17,
      20,    37,    31,    40,    41,    38,     4,     5,     6,     7,
       8,    19,    10,    50,    12,    13,    14,    15,    16,    17,
      31,    20,    25,     9,    25,     4,    24,     6,     7,    27,
      22,    11,    30,    31,    38,    32,    15,    16,    17,    63,
     107,   109,    40,    41,   169,   110,    25,     4,   108,     6,
       7,   121,    50,   124,    11,    -1,   111,   118,    15,    16,
      17,    18,    44,    45,    46,    47,    48,    49,   161
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     4,     6,     7,    11,    15,    16,    17,    18,    56,
      57,    58,    59,    60,    61,    66,    67,    32,     0,    57,
       3,    31,    31,    24,    27,    62,    25,    65,    66,    68,
      28,    63,    64,    65,    66,    67,    27,    69,    21,    31,
      25,    28,    63,    19,    20,    31,     5,     8,    10,    12,
      13,    14,    20,    24,    28,    30,    31,    40,    41,    50,
      65,    69,    70,    71,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    85,    87,    89,    90,    91,    92,
      94,    95,    96,    97,    68,    69,    75,    76,    24,    24,
      76,    24,    76,    26,    89,    89,    89,    20,    28,    70,
      20,    19,    24,    29,    42,    43,    20,    38,    39,    36,
      35,    37,    44,    45,    46,    47,    48,    49,    84,    33,
      34,    83,    50,    51,    86,    52,    53,    54,    88,    20,
      20,    65,    69,    72,    73,    76,    94,    95,    96,    97,
      76,    76,    25,    31,    76,    25,    76,    93,    31,    77,
      78,    79,    80,    81,    82,    85,    87,    89,    20,    25,
      25,    21,    25,    22,    76,    71,    71,    93,    76,    20,
       9,    23,    72,    71,    25,    71
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
#line 77 "ffscript.ypp"
    {resAST = new ASTProgram((ASTDeclList *)(yyvsp[0]), (yylsp[0]));;}
    break;

  case 3:
#line 78 "ffscript.ypp"
    {resAST = new ASTProgram(new ASTDeclList(noloc),noloc);;}
    break;

  case 4:
#line 81 "ffscript.ypp"
    {ASTDeclList *list = (ASTDeclList *)(yyvsp[0]);
				list->addDeclaration((ASTDecl *)(yyvsp[-1]));
				(yyval) = list;;}
    break;

  case 5:
#line 84 "ffscript.ypp"
    {ASTDeclList *list = new ASTDeclList((yylsp[0]));
			  list->addDeclaration((ASTDecl *)(yyvsp[0]));
			  (yyval)=list;;}
    break;

  case 6:
#line 89 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 7:
#line 90 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 8:
#line 91 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 9:
#line 94 "ffscript.ypp"
    {ASTType *type = (ASTType *)(yyvsp[-3]);
								   ASTString *name = (ASTString *)(yyvsp[-1]);
								   ASTDeclList *sblock = (ASTDeclList *)(yyvsp[0]);
								   (yyval) = new ASTScript(type, name->getValue(), sblock,(yylsp[-3]));
								   delete name;;}
    break;

  case 10:
#line 101 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 11:
#line 102 "ffscript.ypp"
    {(yyval) = new ASTTypeGlobal((yylsp[0]));;}
    break;

  case 12:
#line 105 "ffscript.ypp"
    {ASTString *str = (ASTString *)(yyvsp[0]);
					 (yyval) = new ASTImportDecl(str->getValue(),(yylsp[-1]));
					 delete str;;}
    break;

  case 13:
#line 110 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 14:
#line 111 "ffscript.ypp"
    {(yyval) = new ASTDeclList((yylsp[-1]));;}
    break;

  case 15:
#line 114 "ffscript.ypp"
    {ASTDeclList *dl = (ASTDeclList *)(yyvsp[0]);
							  dl->addDeclaration((ASTDecl *)(yyvsp[-1]));
							  (yyval) = dl;;}
    break;

  case 16:
#line 117 "ffscript.ypp"
    {ASTDeclList *dl = new ASTDeclList((yylsp[0]));
			  dl->addDeclaration((ASTDecl *)(yyvsp[0]));
			  (yyval) = dl;;}
    break;

  case 17:
#line 122 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 18:
#line 123 "ffscript.ypp"
    {ASTVarDecl *vd = (ASTVarDecl *)(yyvsp[-3]);
						   Clone c;
						   vd->getType()->execute(c,NULL);
						   (yyval) = new ASTVarDeclInitializer((ASTType *)c.getResult(), vd->getName(), (ASTExpr *)(yyvsp[-1]),(yylsp[-3]));
						   delete vd;;}
    break;

  case 19:
#line 128 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 20:
#line 131 "ffscript.ypp"
    {ASTType *type = (ASTType *)(yyvsp[-1]);
				   ASTString *name = (ASTString *)(yyvsp[0]);
				   (yyval) = new ASTVarDecl(type, name->getValue(),(yylsp[-1]));
				   delete name;;}
    break;

  case 21:
#line 137 "ffscript.ypp"
    {(yyval) = new ASTTypeFloat((yylsp[0]));;}
    break;

  case 22:
#line 138 "ffscript.ypp"
    {(yyval) = new ASTTypeBool((yylsp[0]));;}
    break;

  case 23:
#line 139 "ffscript.ypp"
    {(yyval) = new ASTTypeVoid((yylsp[0]));;}
    break;

  case 24:
#line 140 "ffscript.ypp"
    {(yyval) = new ASTTypeFFC((yylsp[0]));;}
    break;

  case 25:
#line 141 "ffscript.ypp"
    {(yyval) = new ASTTypeItem((yylsp[0]));;}
    break;

  case 26:
#line 142 "ffscript.ypp"
    {(yyval) = new ASTTypeItemclass((yylsp[0]));;}
    break;

  case 27:
#line 145 "ffscript.ypp"
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

  case 28:
#line 154 "ffscript.ypp"
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

  case 29:
#line 165 "ffscript.ypp"
    {ASTFuncDecl *fd = (ASTFuncDecl *)(yyvsp[0]);
						 fd->addParam((ASTVarDecl *)(yyvsp[-2]));
						 (yyval)=fd;;}
    break;

  case 30:
#line 168 "ffscript.ypp"
    {ASTFuncDecl *fd = new ASTFuncDecl((yylsp[0]));
		     fd->addParam((ASTVarDecl *)(yyvsp[0]));
		     (yyval)=fd;;}
    break;

  case 31:
#line 173 "ffscript.ypp"
    {(yyval)=(yyvsp[-1]);;}
    break;

  case 32:
#line 174 "ffscript.ypp"
    {(yyval) = new ASTBlock((yylsp[-1]));;}
    break;

  case 33:
#line 177 "ffscript.ypp"
    {ASTBlock *block = (ASTBlock *)(yyvsp[0]);
				  ASTStmt *stmt = (ASTStmt *)(yyvsp[-1]);
				  block->addStatement(stmt);
				  (yyval) = block;;}
    break;

  case 34:
#line 181 "ffscript.ypp"
    {ASTBlock *block = new ASTBlock((yylsp[0]));
		  ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
		  block->addStatement(stmt);
		  (yyval) = block;;}
    break;

  case 35:
#line 187 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 36:
#line 188 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 37:
#line 189 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 38:
#line 190 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 39:
#line 191 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 40:
#line 192 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 41:
#line 193 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 42:
#line 194 "ffscript.ypp"
    {(yyval) = new ASTStmtEmpty((yylsp[0]));;}
    break;

  case 43:
#line 195 "ffscript.ypp"
    {(yyval)=(yyvsp[-1]);;}
    break;

  case 44:
#line 198 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 45:
#line 199 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 46:
#line 200 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 47:
#line 201 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 48:
#line 202 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 49:
#line 203 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 50:
#line 204 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 51:
#line 205 "ffscript.ypp"
    {(yyval) = new ASTStmtEmpty(noloc);;}
    break;

  case 52:
#line 206 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 53:
#line 209 "ffscript.ypp"
    {(yyval) = new ASTStmtAssign((ASTStmt *)(yyvsp[-2]), (ASTExpr *)(yyvsp[0]),(yylsp[-2]));;}
    break;

  case 54:
#line 212 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 55:
#line 213 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 56:
#line 216 "ffscript.ypp"
    {ASTString *lval = (ASTString *)(yyvsp[-2]);
						 ASTString *rval = (ASTString *)(yyvsp[0]);
						 (yyval) = new ASTExprDot(lval->getValue(), rval->getValue(),(yylsp[-2]));
						 delete lval; delete rval;;}
    break;

  case 57:
#line 220 "ffscript.ypp"
    {ASTString *rval = (ASTString *)(yyvsp[0]);
				  (yyval) = new ASTExprDot("", rval->getValue(),(yylsp[0]));
				  delete rval;;}
    break;

  case 58:
#line 223 "ffscript.ypp"
    {ASTExpr *id = (ASTExpr *)(yyvsp[-2]);
								ASTString *rval = (ASTString *)(yyvsp[0]);
								(yyval) = new ASTExprArrow(id, rval->getValue(), (yylsp[-2]));
								delete rval;;}
    break;

  case 59:
#line 227 "ffscript.ypp"
    {ASTExpr *id = (ASTExpr *)(yyvsp[-5]);
											       	   ASTString *rval = (ASTString *)(yyvsp[-3]);
													   ASTExpr *num = (ASTExpr *)(yyvsp[-1]);
													   ASTExprArrow *res = new ASTExprArrow(id, rval->getValue(), (yylsp[-5]));
													   res->setIndex(num);
													   (yyval) = res;;}
    break;

  case 60:
#line 235 "ffscript.ypp"
    {ASTLogExpr *e = new ASTExprOr((yylsp[-1]));
				 ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				 ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				 e->setFirstOperand(left);
				 e->setSecondOperand(right);
				 (yyval)=e;;}
    break;

  case 61:
#line 241 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 62:
#line 244 "ffscript.ypp"
    {ASTLogExpr *e = new ASTExprAnd((yylsp[-1]));
				 ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				 ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				 e->setFirstOperand(left);
				 e->setSecondOperand(right);
				 (yyval)=e;;}
    break;

  case 63:
#line 250 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 64:
#line 253 "ffscript.ypp"
    {ASTBitExpr *e = new ASTExprBitOr((yylsp[-1]));
							  ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							  ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							  e->setFirstOperand(left);
							  e->setSecondOperand(right);
							  (yyval)=e;;}
    break;

  case 65:
#line 259 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 66:
#line 262 "ffscript.ypp"
    {ASTBitExpr *e = new ASTExprBitXor((yylsp[-1]));
							   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							   e->setFirstOperand(left);
							   e->setSecondOperand(right);
							   (yyval)=e;;}
    break;

  case 67:
#line 268 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 68:
#line 271 "ffscript.ypp"
    {ASTBitExpr *e = new ASTExprBitAnd((yylsp[-1]));
							   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							   e->setFirstOperand(left);
							   e->setSecondOperand(right);
							   (yyval)=e;;}
    break;

  case 69:
#line 277 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 70:
#line 280 "ffscript.ypp"
    {ASTRelExpr *e = (ASTRelExpr *)(yyvsp[-1]);
				   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				   e->setFirstOperand(left);
				   e->setSecondOperand(right);
				   (yyval)=e;;}
    break;

  case 71:
#line 286 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 72:
#line 289 "ffscript.ypp"
    {ASTShiftExpr *e = (ASTShiftExpr *)(yyvsp[-1]);
							   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							   e->setFirstOperand(left);
							   e->setSecondOperand(right);
							   (yyval)=e;;}
    break;

  case 73:
#line 295 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 74:
#line 298 "ffscript.ypp"
    {(yyval) = new ASTExprLShift((yylsp[0]));;}
    break;

  case 75:
#line 299 "ffscript.ypp"
    {(yyval) = new ASTExprRShift((yylsp[0]));;}
    break;

  case 76:
#line 302 "ffscript.ypp"
    {(yyval) = new ASTExprGT((yylsp[0]));;}
    break;

  case 77:
#line 303 "ffscript.ypp"
    {(yyval) = new ASTExprGE((yylsp[0]));;}
    break;

  case 78:
#line 304 "ffscript.ypp"
    {(yyval) = new ASTExprLT((yylsp[0]));;}
    break;

  case 79:
#line 305 "ffscript.ypp"
    {(yyval) = new ASTExprLE((yylsp[0]));;}
    break;

  case 80:
#line 306 "ffscript.ypp"
    {(yyval) = new ASTExprEQ((yylsp[0]));;}
    break;

  case 81:
#line 307 "ffscript.ypp"
    {(yyval) = new ASTExprNE((yylsp[0]));;}
    break;

  case 82:
#line 310 "ffscript.ypp"
    {ASTAddExpr *e = (ASTAddExpr *)(yyvsp[-1]);
				   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				   e->setFirstOperand(left);
				   e->setSecondOperand(right);
				   (yyval)=e;;}
    break;

  case 83:
#line 316 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 84:
#line 319 "ffscript.ypp"
    {(yyval) = new ASTExprPlus((yylsp[0]));;}
    break;

  case 85:
#line 320 "ffscript.ypp"
    {(yyval) = new ASTExprMinus((yylsp[0]));;}
    break;

  case 86:
#line 323 "ffscript.ypp"
    {ASTMultExpr *e = (ASTMultExpr *)(yyvsp[-1]);
				    ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				    ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				    e->setFirstOperand(left);
				    e->setSecondOperand(right);
				    (yyval)=e;;}
    break;

  case 87:
#line 329 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 88:
#line 332 "ffscript.ypp"
    {(yyval) = new ASTExprTimes((yylsp[0]));;}
    break;

  case 89:
#line 333 "ffscript.ypp"
    {(yyval) = new ASTExprDivide((yylsp[0]));;}
    break;

  case 90:
#line 334 "ffscript.ypp"
    {(yyval) = new ASTExprModulo((yylsp[0]));;}
    break;

  case 91:
#line 337 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprNot((yylsp[-1]));
			 ASTExpr *op = (ASTExpr *)(yyvsp[0]);
			 e->setOperand(op);
			 (yyval)=e;;}
    break;

  case 92:
#line 341 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprNegate((yylsp[-1]));
			   ASTExpr *op = (ASTExpr *)(yyvsp[0]);
			   e->setOperand(op);
			   (yyval)=e;;}
    break;

  case 93:
#line 345 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprBitNot((yylsp[-1]));
					ASTExpr *op = (ASTExpr *)(yyvsp[0]);
					e->setOperand(op);
					(yyval)=e;;}
    break;

  case 94:
#line 349 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 95:
#line 352 "ffscript.ypp"
    {(yyval)=(yyvsp[-1]);;}
    break;

  case 96:
#line 353 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 97:
#line 354 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprIncrement((yylsp[0]));
						 ASTExpr *op = (ASTExpr *)(yyvsp[-1]);
						 e->setOperand(op);
						 (yyval)=e;;}
    break;

  case 98:
#line 358 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprDecrement((yylsp[0]));
						 ASTExpr *op = (ASTExpr *)(yyvsp[-1]);
						 e->setOperand(op);
						 (yyval)=e;;}
    break;

  case 99:
#line 362 "ffscript.ypp"
    {ASTFloat *val = (ASTFloat *)(yyvsp[0]);
		    (yyval) = new ASTNumConstant(val,(yylsp[0]));;}
    break;

  case 100:
#line 364 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 101:
#line 365 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 102:
#line 368 "ffscript.ypp"
    {(yyval) = new ASTBoolConstant(true,(yylsp[0]));;}
    break;

  case 103:
#line 369 "ffscript.ypp"
    {(yyval) = new ASTBoolConstant(false,(yylsp[0]));;}
    break;

  case 104:
#line 372 "ffscript.ypp"
    {ASTFuncCall *fc = (ASTFuncCall *)(yyvsp[-1]);
							    ASTExpr *name = (ASTExpr *)(yyvsp[-3]);
							    fc->setName(name);
							    (yyval)=fc;;}
    break;

  case 105:
#line 376 "ffscript.ypp"
    {ASTFuncCall *fc = new ASTFuncCall((yylsp[-2]));
					    ASTExpr *name = (ASTExpr *)(yyvsp[-2]);
					    fc->setName(name);
					    (yyval)=fc;;}
    break;

  case 106:
#line 382 "ffscript.ypp"
    {ASTFuncCall *fc = (ASTFuncCall *)(yyvsp[0]);
					  ASTExpr *e = (ASTExpr *)(yyvsp[-2]);
					  fc->addParam(e);
					  (yyval) = fc;;}
    break;

  case 107:
#line 386 "ffscript.ypp"
    {ASTFuncCall *fc = new ASTFuncCall((yylsp[0]));
		  ASTExpr *e = (ASTExpr *)(yyvsp[0]);
		  fc->addParam(e);
		  (yyval) = fc;;}
    break;

  case 108:
#line 392 "ffscript.ypp"
    {ASTStmt *prec = (ASTStmt *)(yyvsp[-6]);
												ASTExpr *term = (ASTExpr *)(yyvsp[-4]);
												ASTStmt *incr = (ASTExpr *)(yyvsp[-2]);
												ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
												(yyval) = new ASTStmtFor(prec,term,incr,stmt,(yylsp[-8]));;}
    break;

  case 109:
#line 399 "ffscript.ypp"
    {ASTExpr *cond = (ASTExpr *)(yyvsp[-2]);
										   ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
										   (yyval) = new ASTStmtWhile(cond,stmt,(yylsp[-4]));;}
    break;

  case 110:
#line 403 "ffscript.ypp"
    {ASTExpr *cond = (ASTExpr *)(yyvsp[-2]);
							  ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
							  (yyval) = new ASTStmtIf(cond,stmt,(yylsp[-4]));;}
    break;

  case 111:
#line 406 "ffscript.ypp"
    {ASTExpr *cond = (ASTExpr *)(yyvsp[-4]);
										ASTStmt *ifstmt = (ASTStmt *)(yyvsp[-2]);
										ASTStmt *elsestmt = (ASTStmt *)(yyvsp[0]);
										(yyval) = new ASTStmtIfElse(cond,ifstmt,elsestmt,(yylsp[-6]));;}
    break;

  case 112:
#line 412 "ffscript.ypp"
    {(yyval) = new ASTStmtReturnVal((ASTExpr *)(yyvsp[0]),(yylsp[-1]));;}
    break;

  case 113:
#line 413 "ffscript.ypp"
    {(yyval) = new ASTStmtReturn((yylsp[0]));;}
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 2139 "y.tab.cpp"

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


#line 416 "ffscript.ypp"


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

