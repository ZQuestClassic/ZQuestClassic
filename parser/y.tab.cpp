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
#line 240 "y.tab.cpp"

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
#define YYFINAL  21
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   262

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  58
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  44
/* YYNRULES -- Number of rules. */
#define YYNRULES  119
/* YYNRULES -- Number of states. */
#define YYNSTATES  189

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   312

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
      55,    56,    57
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    11,    13,    15,    17,
      19,    26,    31,    33,    35,    38,    42,    45,    48,    50,
      53,    58,    60,    63,    65,    67,    69,    71,    73,    75,
      82,    88,    92,    94,    98,   101,   104,   106,   109,   112,
     114,   116,   118,   121,   123,   125,   128,   131,   134,   136,
     138,   140,   142,   144,   146,   148,   149,   151,   153,   155,
     159,   161,   163,   167,   169,   173,   180,   184,   186,   190,
     192,   196,   198,   202,   204,   208,   210,   214,   216,   220,
     222,   224,   226,   228,   230,   232,   234,   236,   238,   242,
     244,   246,   248,   252,   254,   256,   258,   260,   263,   266,
     269,   271,   275,   277,   280,   283,   285,   287,   289,   291,
     293,   298,   302,   306,   308,   318,   324,   330,   338,   341
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      59,     0,    -1,    60,    -1,    -1,    61,    60,    -1,    61,
      -1,    63,    -1,    65,    -1,    71,    -1,    62,    -1,    21,
       4,    34,    22,    33,    23,    -1,    64,     3,    34,    66,
      -1,    70,    -1,    18,    -1,    11,    35,    -1,    30,    67,
      31,    -1,    30,    31,    -1,    68,    67,    -1,    68,    -1,
      69,    23,    -1,    69,    22,    80,    23,    -1,    71,    -1,
      70,    34,    -1,     4,    -1,     6,    -1,     7,    -1,    15,
      -1,    16,    -1,    17,    -1,    70,    34,    27,    72,    28,
      73,    -1,    70,    34,    27,    28,    73,    -1,    69,    24,
      72,    -1,    69,    -1,    30,    74,    31,    -1,    30,    31,
      -1,    75,    74,    -1,    75,    -1,    69,    23,    -1,    77,
      23,    -1,    98,    -1,   100,    -1,    73,    -1,   101,    23,
      -1,    99,    -1,    23,    -1,    80,    23,    -1,    19,    23,
      -1,    20,    23,    -1,    69,    -1,    77,    -1,    98,    -1,
     100,    -1,    73,    -1,   101,    -1,    99,    -1,    -1,    80,
      -1,    19,    -1,    20,    -1,    78,    22,    80,    -1,    69,
      -1,    79,    -1,    34,    29,    34,    -1,    34,    -1,    79,
      32,    34,    -1,    79,    32,    34,    25,    80,    26,    -1,
      80,    41,    81,    -1,    81,    -1,    81,    42,    82,    -1,
      82,    -1,    82,    39,    83,    -1,    83,    -1,    83,    38,
      84,    -1,    84,    -1,    84,    40,    85,    -1,    85,    -1,
      85,    88,    86,    -1,    86,    -1,    86,    87,    89,    -1,
      89,    -1,    37,    -1,    36,    -1,    49,    -1,    50,    -1,
      51,    -1,    52,    -1,    48,    -1,    47,    -1,    89,    90,
      91,    -1,    91,    -1,    54,    -1,    53,    -1,    91,    92,
      93,    -1,    93,    -1,    56,    -1,    55,    -1,    57,    -1,
      44,    93,    -1,    53,    93,    -1,    43,    93,    -1,    94,
      -1,    27,    80,    28,    -1,    79,    -1,    79,    46,    -1,
      79,    45,    -1,    33,    -1,    95,    -1,    96,    -1,    12,
      -1,    13,    -1,    79,    27,    97,    28,    -1,    79,    27,
      28,    -1,    80,    24,    97,    -1,    80,    -1,     5,    27,
      76,    23,    80,    23,    76,    28,    75,    -1,    14,    27,
      80,    28,    75,    -1,     8,    27,    80,    28,    75,    -1,
       8,    27,    80,    28,    75,     9,    75,    -1,    10,    80,
      -1,    10,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,    80,    80,    81,    84,    87,    92,    93,    94,    95,
      98,   103,   110,   111,   114,   119,   120,   123,   126,   131,
     132,   137,   140,   146,   147,   148,   149,   150,   151,   154,
     163,   174,   177,   182,   183,   186,   190,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   222,
     225,   226,   229,   233,   236,   240,   248,   254,   257,   263,
     266,   272,   275,   281,   284,   290,   293,   299,   302,   308,
     311,   312,   315,   316,   317,   318,   319,   320,   323,   329,
     332,   333,   336,   342,   345,   346,   347,   350,   354,   358,
     362,   365,   366,   367,   371,   375,   377,   378,   381,   382,
     385,   389,   395,   399,   405,   412,   416,   419,   425,   426
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SCRIPT", "FLOAT", "FOR", "BOOL", "VOID",
  "IF", "ELSE", "RETURN", "IMPORT", "TRUE", "FALSE", "WHILE", "FFC",
  "ITEM", "ITEMCLASS", "GLOBAL", "BREAK", "CONTINUE", "CONST", "ASSIGN",
  "SEMICOLON", "COMMA", "LBRACKET", "RBRACKET", "LPAREN", "RPAREN", "DOT",
  "LBRACE", "RBRACE", "ARROW", "NUMBER", "IDENTIFIER", "QUOTEDSTRING",
  "RSHIFT", "LSHIFT", "BITXOR", "BITOR", "BITAND", "OR", "AND", "BITNOT",
  "NOT", "DECREMENT", "INCREMENT", "NE", "EQ", "GT", "GE", "LT", "LE",
  "MINUS", "PLUS", "DIVIDE", "TIMES", "MODULO", "$accept", "Init", "Prog",
  "GlobalStmt", "ConstDecl", "Script", "ScriptType", "Import",
  "ScriptBlock", "ScriptStmtList", "ScriptStmt", "VarDecl", "Type",
  "FuncDecl", "ParamList", "Block", "StmtList", "Stmt", "StmtNoSemi",
  "AssignStmt", "LVal", "DotExpr", "Expr", "Expr15", "Expr16", "Expr17",
  "Expr18", "Expr2", "Expr25", "ShiftOp", "RelOp", "Expr3", "AddOp",
  "Expr4", "MultOp", "Expr5", "Factor", "BoolConstant", "FuncCall",
  "ExprList", "ForStmt", "WhileStmt", "IfStmt", "ReturnStmt", 0
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
     305,   306,   307,   308,   309,   310,   311,   312
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    58,    59,    59,    60,    60,    61,    61,    61,    61,
      62,    63,    64,    64,    65,    66,    66,    67,    67,    68,
      68,    68,    69,    70,    70,    70,    70,    70,    70,    71,
      71,    72,    72,    73,    73,    74,    74,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    77,
      78,    78,    79,    79,    79,    79,    80,    80,    81,    81,
      82,    82,    83,    83,    84,    84,    85,    85,    86,    86,
      87,    87,    88,    88,    88,    88,    88,    88,    89,    89,
      90,    90,    91,    91,    92,    92,    92,    93,    93,    93,
      93,    94,    94,    94,    94,    94,    94,    94,    95,    95,
      96,    96,    97,    97,    98,    99,   100,   100,   101,   101
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     0,     2,     1,     1,     1,     1,     1,
       6,     4,     1,     1,     2,     3,     2,     2,     1,     2,
       4,     1,     2,     1,     1,     1,     1,     1,     1,     6,
       5,     3,     1,     3,     2,     2,     1,     2,     2,     1,
       1,     1,     2,     1,     1,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     1,     1,     3,
       1,     1,     3,     1,     3,     6,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     3,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     3,     1,     1,     1,     1,     2,     2,     2,
       1,     3,     1,     2,     2,     1,     1,     1,     1,     1,
       4,     3,     3,     1,     9,     5,     5,     7,     2,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       3,    23,    24,    25,     0,    26,    27,    28,    13,     0,
       0,     2,     5,     9,     6,     0,     7,    12,     8,    14,
       0,     1,     4,     0,     0,     0,     0,     0,     0,     0,
      11,     0,    32,     0,     0,     0,    16,     0,    18,     0,
       0,    21,     0,    30,     0,    22,     0,    10,    15,    17,
       0,    19,    22,     0,     0,   119,   108,   109,     0,     0,
       0,    44,     0,    34,   105,    63,     0,     0,     0,    60,
      41,     0,    36,     0,     0,   102,     0,    67,    69,    71,
      73,    75,    77,    79,    89,    93,   100,   106,   107,    39,
      43,    40,     0,    31,    29,   102,     0,    55,     0,   118,
       0,    46,    47,     0,     0,    99,    97,    98,    37,    33,
      35,    38,     0,     0,     0,   104,   103,    45,     0,     0,
       0,     0,     0,    87,    86,    82,    83,    84,    85,     0,
      81,    80,     0,    91,    90,     0,    95,    94,    96,     0,
      42,    20,    57,    58,    48,    52,     0,    49,    56,    50,
      54,    51,    53,     0,     0,   101,    62,    59,   111,   113,
       0,    64,    66,    68,    70,    72,    74,    76,    78,    88,
      92,     0,     0,     0,     0,   110,     0,     0,   116,   115,
     112,     0,    55,     0,    65,     0,   117,     0,   114
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,    10,    11,    12,    13,    14,    15,    16,    30,    37,
      38,    69,    33,    18,    34,    70,    71,    72,   146,    73,
      74,    95,    76,    77,    78,    79,    80,    81,    82,   132,
     129,    83,   135,    84,   139,    85,    86,    87,    88,   160,
      89,    90,    91,    92
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -94
static const short int yypact[] =
{
     235,   -94,   -94,   -94,     7,   -94,   -94,   -94,   -94,    44,
      57,   -94,   235,   -94,   -94,    71,   -94,    49,   -94,   -94,
      51,   -94,   -94,    61,    80,    56,    79,   216,    83,    65,
     -94,    89,    96,    92,    99,   107,   -94,   105,   174,   -10,
     103,   -94,    98,   -94,   174,   -94,    89,   -94,   -94,   -94,
      22,   -94,    80,   112,   118,    22,   -94,   -94,   120,   126,
     127,   -94,    22,   -94,   -94,   124,    22,    22,    22,   131,
     -94,   130,   152,   140,   148,    41,    -5,   134,   138,   145,
     147,   210,    23,    46,    67,   -94,   -94,   -94,   -94,   -94,
     -94,   -94,   169,   -94,   -94,    52,     5,   194,    22,   153,
      22,   -94,   -94,     4,   159,   -94,   -94,   -94,   -94,   -94,
     -94,   -94,    22,    -3,   163,   -94,   -94,   -94,    22,    22,
      22,    22,    22,   -94,   -94,   -94,   -94,   -94,   -94,    22,
     -94,   -94,    22,   -94,   -94,    22,   -94,   -94,   -94,    22,
     -94,   -94,   -94,   -94,   181,   -94,   189,   -94,   153,   -94,
     -94,   -94,   -94,    10,    60,   -94,   -94,   153,   -94,    -4,
     187,   191,   134,   138,   145,   147,   210,    23,    46,    67,
     -94,    22,   152,   152,    22,   -94,    22,     6,   208,   -94,
     -94,    13,   194,   152,   -94,   190,   -94,   152,   -94
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
     -94,   -94,   207,   -94,   -94,   -94,   -94,   -94,   -94,   188,
     -94,   -27,    15,    14,   185,   -30,   158,     1,    43,   -93,
     -94,   -39,   -36,   116,   117,   115,   119,   121,   125,   -94,
     -94,   113,   -94,   114,   -94,   -45,   -94,   -94,   -94,    74,
     -92,   -91,   -90,   -89
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -62
static const short int yytable[] =
{
      32,    43,    39,    75,   147,   149,   150,   151,   152,    56,
      57,    39,    50,    51,    96,    17,    94,    32,   117,    99,
     174,   105,   106,   107,    62,   158,   103,    17,   141,   182,
      64,    65,   155,    75,    56,    57,   118,   118,   172,   184,
      66,    67,    19,    41,    40,   118,   118,   118,    20,    62,
      68,   118,    41,    40,   118,    64,    65,    21,    75,   130,
     131,   148,   153,   -61,   154,    66,    67,   145,   113,     1,
     144,     2,     3,   114,    23,    68,   157,   159,    28,   113,
       5,     6,     7,    24,   114,    25,   115,   116,   173,   147,
     149,   150,   151,   152,   170,    26,    36,   115,   116,   133,
     134,   118,     1,    53,     2,     3,    54,    27,    55,    29,
      56,    57,    58,     5,     6,     7,    35,    59,    60,    42,
      44,    61,   136,   137,   138,    62,    45,    46,    42,    63,
      47,    64,    65,    75,    75,   177,    48,    52,   159,    97,
     181,    66,    67,    75,    75,    98,   148,   100,    75,   101,
     102,    68,   145,   104,   108,   144,     1,    53,     2,     3,
      54,   109,    55,   111,    56,    57,    58,     5,     6,     7,
     112,    59,    60,   178,   179,    61,   119,   120,     1,    62,
       2,     3,    42,   121,   186,    64,    65,   122,   188,     5,
       6,     7,   140,   156,   118,    66,    67,   161,     1,    53,
       2,     3,    54,   -60,    55,    68,    56,    57,    58,     5,
       6,     7,   171,   142,   143,   175,   176,   183,   187,    22,
       1,    62,     2,     3,    42,   185,    49,    64,    65,    93,
     110,     5,     6,     7,   162,   164,   163,    66,    67,     1,
     165,     2,     3,   166,    31,   168,     4,    68,   180,   169,
       5,     6,     7,     8,   167,     0,     9,   123,   124,   125,
     126,   127,   128
};

static const short int yycheck[] =
{
      27,    31,    29,    42,    97,    97,    97,    97,    97,    12,
      13,    38,    22,    23,    50,     0,    46,    44,    23,    55,
      24,    66,    67,    68,    27,    28,    62,    12,    23,    23,
      33,    34,    28,    72,    12,    13,    41,    41,    28,    26,
      43,    44,    35,    29,    29,    41,    41,    41,     4,    27,
      53,    41,    38,    38,    41,    33,    34,     0,    97,    36,
      37,    97,    98,    22,   100,    43,    44,    97,    27,     4,
      97,     6,     7,    32,     3,    53,   112,   113,    22,    27,
      15,    16,    17,    34,    32,    34,    45,    46,    28,   182,
     182,   182,   182,   182,   139,    34,    31,    45,    46,    53,
      54,    41,     4,     5,     6,     7,     8,    27,    10,    30,
      12,    13,    14,    15,    16,    17,    33,    19,    20,    30,
      24,    23,    55,    56,    57,    27,    34,    28,    30,    31,
      23,    33,    34,   172,   173,   171,    31,    34,   174,    27,
     176,    43,    44,   182,   183,    27,   182,    27,   187,    23,
      23,    53,   182,    29,    23,   182,     4,     5,     6,     7,
       8,    31,    10,    23,    12,    13,    14,    15,    16,    17,
      22,    19,    20,   172,   173,    23,    42,    39,     4,    27,
       6,     7,    30,    38,   183,    33,    34,    40,   187,    15,
      16,    17,    23,    34,    41,    43,    44,    34,     4,     5,
       6,     7,     8,    22,    10,    53,    12,    13,    14,    15,
      16,    17,    23,    19,    20,    28,    25,     9,    28,    12,
       4,    27,     6,     7,    30,   182,    38,    33,    34,    44,
      72,    15,    16,    17,   118,   120,   119,    43,    44,     4,
     121,     6,     7,   122,    28,   132,    11,    53,   174,   135,
      15,    16,    17,    18,   129,    -1,    21,    47,    48,    49,
      50,    51,    52
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     4,     6,     7,    11,    15,    16,    17,    18,    21,
      59,    60,    61,    62,    63,    64,    65,    70,    71,    35,
       4,     0,    60,     3,    34,    34,    34,    27,    22,    30,
      66,    28,    69,    70,    72,    33,    31,    67,    68,    69,
      70,    71,    30,    73,    24,    34,    28,    23,    31,    67,
      22,    23,    34,     5,     8,    10,    12,    13,    14,    19,
      20,    23,    27,    31,    33,    34,    43,    44,    53,    69,
      73,    74,    75,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    89,    91,    93,    94,    95,    96,    98,
      99,   100,   101,    72,    73,    79,    80,    27,    27,    80,
      27,    23,    23,    80,    29,    93,    93,    93,    23,    31,
      74,    23,    22,    27,    32,    45,    46,    23,    41,    42,
      39,    38,    40,    47,    48,    49,    50,    51,    52,    88,
      36,    37,    87,    53,    54,    90,    55,    56,    57,    92,
      23,    23,    19,    20,    69,    73,    76,    77,    80,    98,
      99,   100,   101,    80,    80,    28,    34,    80,    28,    80,
      97,    34,    81,    82,    83,    84,    85,    86,    89,    91,
      93,    23,    28,    28,    24,    28,    25,    80,    75,    75,
      97,    80,    23,     9,    26,    76,    75,    28,    75
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
#line 80 "ffscript.ypp"
    {resAST = new ASTProgram((ASTDeclList *)(yyvsp[0]), (yylsp[0]));;}
    break;

  case 3:
#line 81 "ffscript.ypp"
    {resAST = new ASTProgram(new ASTDeclList(noloc),noloc);;}
    break;

  case 4:
#line 84 "ffscript.ypp"
    {ASTDeclList *list = (ASTDeclList *)(yyvsp[0]);
				list->addDeclaration((ASTDecl *)(yyvsp[-1]));
				(yyval) = list;;}
    break;

  case 5:
#line 87 "ffscript.ypp"
    {ASTDeclList *list = new ASTDeclList((yylsp[0]));
			  list->addDeclaration((ASTDecl *)(yyvsp[0]));
			  (yyval)=list;;}
    break;

  case 6:
#line 92 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 7:
#line 93 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 8:
#line 94 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 9:
#line 95 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 10:
#line 98 "ffscript.ypp"
    {ASTString *name = (ASTString *)(yyvsp[-3]);
															ASTFloat *val = (ASTFloat *)(yyvsp[-1]);
															(yyval) = new ASTConstDecl(name->getValue(), val,(yylsp[-5]));
															delete name;;}
    break;

  case 11:
#line 103 "ffscript.ypp"
    {ASTType *type = (ASTType *)(yyvsp[-3]);
								   ASTString *name = (ASTString *)(yyvsp[-1]);
								   ASTDeclList *sblock = (ASTDeclList *)(yyvsp[0]);
								   (yyval) = new ASTScript(type, name->getValue(), sblock,(yylsp[-3]));
								   delete name;;}
    break;

  case 12:
#line 110 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 13:
#line 111 "ffscript.ypp"
    {(yyval) = new ASTTypeGlobal((yylsp[0]));;}
    break;

  case 14:
#line 114 "ffscript.ypp"
    {ASTString *str = (ASTString *)(yyvsp[0]);
					 (yyval) = new ASTImportDecl(str->getValue(),(yylsp[-1]));
					 delete str;;}
    break;

  case 15:
#line 119 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 16:
#line 120 "ffscript.ypp"
    {(yyval) = new ASTDeclList((yylsp[-1]));;}
    break;

  case 17:
#line 123 "ffscript.ypp"
    {ASTDeclList *dl = (ASTDeclList *)(yyvsp[0]);
							  dl->addDeclaration((ASTDecl *)(yyvsp[-1]));
							  (yyval) = dl;;}
    break;

  case 18:
#line 126 "ffscript.ypp"
    {ASTDeclList *dl = new ASTDeclList((yylsp[0]));
			  dl->addDeclaration((ASTDecl *)(yyvsp[0]));
			  (yyval) = dl;;}
    break;

  case 19:
#line 131 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 20:
#line 132 "ffscript.ypp"
    {ASTVarDecl *vd = (ASTVarDecl *)(yyvsp[-3]);
						   Clone c;
						   vd->getType()->execute(c,NULL);
						   (yyval) = new ASTVarDeclInitializer((ASTType *)c.getResult(), vd->getName(), (ASTExpr *)(yyvsp[-1]),(yylsp[-3]));
						   delete vd;;}
    break;

  case 21:
#line 137 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 22:
#line 140 "ffscript.ypp"
    {ASTType *type = (ASTType *)(yyvsp[-1]);
				   ASTString *name = (ASTString *)(yyvsp[0]);
				   (yyval) = new ASTVarDecl(type, name->getValue(),(yylsp[-1]));
				   delete name;;}
    break;

  case 23:
#line 146 "ffscript.ypp"
    {(yyval) = new ASTTypeFloat((yylsp[0]));;}
    break;

  case 24:
#line 147 "ffscript.ypp"
    {(yyval) = new ASTTypeBool((yylsp[0]));;}
    break;

  case 25:
#line 148 "ffscript.ypp"
    {(yyval) = new ASTTypeVoid((yylsp[0]));;}
    break;

  case 26:
#line 149 "ffscript.ypp"
    {(yyval) = new ASTTypeFFC((yylsp[0]));;}
    break;

  case 27:
#line 150 "ffscript.ypp"
    {(yyval) = new ASTTypeItem((yylsp[0]));;}
    break;

  case 28:
#line 151 "ffscript.ypp"
    {(yyval) = new ASTTypeItemclass((yylsp[0]));;}
    break;

  case 29:
#line 154 "ffscript.ypp"
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

  case 30:
#line 163 "ffscript.ypp"
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

  case 31:
#line 174 "ffscript.ypp"
    {ASTFuncDecl *fd = (ASTFuncDecl *)(yyvsp[0]);
						 fd->addParam((ASTVarDecl *)(yyvsp[-2]));
						 (yyval)=fd;;}
    break;

  case 32:
#line 177 "ffscript.ypp"
    {ASTFuncDecl *fd = new ASTFuncDecl((yylsp[0]));
		     fd->addParam((ASTVarDecl *)(yyvsp[0]));
		     (yyval)=fd;;}
    break;

  case 33:
#line 182 "ffscript.ypp"
    {(yyval)=(yyvsp[-1]);;}
    break;

  case 34:
#line 183 "ffscript.ypp"
    {(yyval) = new ASTBlock((yylsp[-1]));;}
    break;

  case 35:
#line 186 "ffscript.ypp"
    {ASTBlock *block = (ASTBlock *)(yyvsp[0]);
				  ASTStmt *stmt = (ASTStmt *)(yyvsp[-1]);
				  block->addStatement(stmt);
				  (yyval) = block;;}
    break;

  case 36:
#line 190 "ffscript.ypp"
    {ASTBlock *block = new ASTBlock((yylsp[0]));
		  ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
		  block->addStatement(stmt);
		  (yyval) = block;;}
    break;

  case 37:
#line 196 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 38:
#line 197 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 39:
#line 198 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 40:
#line 199 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 41:
#line 200 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 42:
#line 201 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 43:
#line 202 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 44:
#line 203 "ffscript.ypp"
    {(yyval) = new ASTStmtEmpty((yylsp[0]));;}
    break;

  case 45:
#line 204 "ffscript.ypp"
    {(yyval)=(yyvsp[-1]);;}
    break;

  case 46:
#line 205 "ffscript.ypp"
    {(yyval) = new ASTStmtBreak((yylsp[-1]));;}
    break;

  case 47:
#line 206 "ffscript.ypp"
    {(yyval) = new ASTStmtContinue((yylsp[-1]));;}
    break;

  case 48:
#line 209 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 49:
#line 210 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 50:
#line 211 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 51:
#line 212 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 52:
#line 213 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 53:
#line 214 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 54:
#line 215 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 55:
#line 216 "ffscript.ypp"
    {(yyval) = new ASTStmtEmpty(noloc);;}
    break;

  case 56:
#line 217 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 57:
#line 218 "ffscript.ypp"
    {(yyval) = new ASTStmtBreak((yylsp[0]));;}
    break;

  case 58:
#line 219 "ffscript.ypp"
    {(yyval) = new ASTStmtContinue((yylsp[0]));;}
    break;

  case 59:
#line 222 "ffscript.ypp"
    {(yyval) = new ASTStmtAssign((ASTStmt *)(yyvsp[-2]), (ASTExpr *)(yyvsp[0]),(yylsp[-2]));;}
    break;

  case 60:
#line 225 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 61:
#line 226 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 62:
#line 229 "ffscript.ypp"
    {ASTString *lval = (ASTString *)(yyvsp[-2]);
						 ASTString *rval = (ASTString *)(yyvsp[0]);
						 (yyval) = new ASTExprDot(lval->getValue(), rval->getValue(),(yylsp[-2]));
						 delete lval; delete rval;;}
    break;

  case 63:
#line 233 "ffscript.ypp"
    {ASTString *rval = (ASTString *)(yyvsp[0]);
				  (yyval) = new ASTExprDot("", rval->getValue(),(yylsp[0]));
				  delete rval;;}
    break;

  case 64:
#line 236 "ffscript.ypp"
    {ASTExpr *id = (ASTExpr *)(yyvsp[-2]);
								ASTString *rval = (ASTString *)(yyvsp[0]);
								(yyval) = new ASTExprArrow(id, rval->getValue(), (yylsp[-2]));
								delete rval;;}
    break;

  case 65:
#line 240 "ffscript.ypp"
    {ASTExpr *id = (ASTExpr *)(yyvsp[-5]);
											       	   ASTString *rval = (ASTString *)(yyvsp[-3]);
													   ASTExpr *num = (ASTExpr *)(yyvsp[-1]);
													   ASTExprArrow *res = new ASTExprArrow(id, rval->getValue(), (yylsp[-5]));
													   res->setIndex(num);
													   (yyval) = res;;}
    break;

  case 66:
#line 248 "ffscript.ypp"
    {ASTLogExpr *e = new ASTExprOr((yylsp[-1]));
				 ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				 ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				 e->setFirstOperand(left);
				 e->setSecondOperand(right);
				 (yyval)=e;;}
    break;

  case 67:
#line 254 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 68:
#line 257 "ffscript.ypp"
    {ASTLogExpr *e = new ASTExprAnd((yylsp[-1]));
				 ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				 ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				 e->setFirstOperand(left);
				 e->setSecondOperand(right);
				 (yyval)=e;;}
    break;

  case 69:
#line 263 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 70:
#line 266 "ffscript.ypp"
    {ASTBitExpr *e = new ASTExprBitOr((yylsp[-1]));
							  ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							  ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							  e->setFirstOperand(left);
							  e->setSecondOperand(right);
							  (yyval)=e;;}
    break;

  case 71:
#line 272 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 72:
#line 275 "ffscript.ypp"
    {ASTBitExpr *e = new ASTExprBitXor((yylsp[-1]));
							   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							   e->setFirstOperand(left);
							   e->setSecondOperand(right);
							   (yyval)=e;;}
    break;

  case 73:
#line 281 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 74:
#line 284 "ffscript.ypp"
    {ASTBitExpr *e = new ASTExprBitAnd((yylsp[-1]));
							   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							   e->setFirstOperand(left);
							   e->setSecondOperand(right);
							   (yyval)=e;;}
    break;

  case 75:
#line 290 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 76:
#line 293 "ffscript.ypp"
    {ASTRelExpr *e = (ASTRelExpr *)(yyvsp[-1]);
				   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				   e->setFirstOperand(left);
				   e->setSecondOperand(right);
				   (yyval)=e;;}
    break;

  case 77:
#line 299 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 78:
#line 302 "ffscript.ypp"
    {ASTShiftExpr *e = (ASTShiftExpr *)(yyvsp[-1]);
							   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							   e->setFirstOperand(left);
							   e->setSecondOperand(right);
							   (yyval)=e;;}
    break;

  case 79:
#line 308 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 80:
#line 311 "ffscript.ypp"
    {(yyval) = new ASTExprLShift((yylsp[0]));;}
    break;

  case 81:
#line 312 "ffscript.ypp"
    {(yyval) = new ASTExprRShift((yylsp[0]));;}
    break;

  case 82:
#line 315 "ffscript.ypp"
    {(yyval) = new ASTExprGT((yylsp[0]));;}
    break;

  case 83:
#line 316 "ffscript.ypp"
    {(yyval) = new ASTExprGE((yylsp[0]));;}
    break;

  case 84:
#line 317 "ffscript.ypp"
    {(yyval) = new ASTExprLT((yylsp[0]));;}
    break;

  case 85:
#line 318 "ffscript.ypp"
    {(yyval) = new ASTExprLE((yylsp[0]));;}
    break;

  case 86:
#line 319 "ffscript.ypp"
    {(yyval) = new ASTExprEQ((yylsp[0]));;}
    break;

  case 87:
#line 320 "ffscript.ypp"
    {(yyval) = new ASTExprNE((yylsp[0]));;}
    break;

  case 88:
#line 323 "ffscript.ypp"
    {ASTAddExpr *e = (ASTAddExpr *)(yyvsp[-1]);
				   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				   e->setFirstOperand(left);
				   e->setSecondOperand(right);
				   (yyval)=e;;}
    break;

  case 89:
#line 329 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 90:
#line 332 "ffscript.ypp"
    {(yyval) = new ASTExprPlus((yylsp[0]));;}
    break;

  case 91:
#line 333 "ffscript.ypp"
    {(yyval) = new ASTExprMinus((yylsp[0]));;}
    break;

  case 92:
#line 336 "ffscript.ypp"
    {ASTMultExpr *e = (ASTMultExpr *)(yyvsp[-1]);
				    ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				    ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				    e->setFirstOperand(left);
				    e->setSecondOperand(right);
				    (yyval)=e;;}
    break;

  case 93:
#line 342 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 94:
#line 345 "ffscript.ypp"
    {(yyval) = new ASTExprTimes((yylsp[0]));;}
    break;

  case 95:
#line 346 "ffscript.ypp"
    {(yyval) = new ASTExprDivide((yylsp[0]));;}
    break;

  case 96:
#line 347 "ffscript.ypp"
    {(yyval) = new ASTExprModulo((yylsp[0]));;}
    break;

  case 97:
#line 350 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprNot((yylsp[-1]));
			 ASTExpr *op = (ASTExpr *)(yyvsp[0]);
			 e->setOperand(op);
			 (yyval)=e;;}
    break;

  case 98:
#line 354 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprNegate((yylsp[-1]));
			   ASTExpr *op = (ASTExpr *)(yyvsp[0]);
			   e->setOperand(op);
			   (yyval)=e;;}
    break;

  case 99:
#line 358 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprBitNot((yylsp[-1]));
					ASTExpr *op = (ASTExpr *)(yyvsp[0]);
					e->setOperand(op);
					(yyval)=e;;}
    break;

  case 100:
#line 362 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 101:
#line 365 "ffscript.ypp"
    {(yyval)=(yyvsp[-1]);;}
    break;

  case 102:
#line 366 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 103:
#line 367 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprIncrement((yylsp[0]));
						 ASTExpr *op = (ASTExpr *)(yyvsp[-1]);
						 e->setOperand(op);
						 (yyval)=e;;}
    break;

  case 104:
#line 371 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprDecrement((yylsp[0]));
						 ASTExpr *op = (ASTExpr *)(yyvsp[-1]);
						 e->setOperand(op);
						 (yyval)=e;;}
    break;

  case 105:
#line 375 "ffscript.ypp"
    {ASTFloat *val = (ASTFloat *)(yyvsp[0]);
		    (yyval) = new ASTNumConstant(val,(yylsp[0]));;}
    break;

  case 106:
#line 377 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 107:
#line 378 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 108:
#line 381 "ffscript.ypp"
    {(yyval) = new ASTBoolConstant(true,(yylsp[0]));;}
    break;

  case 109:
#line 382 "ffscript.ypp"
    {(yyval) = new ASTBoolConstant(false,(yylsp[0]));;}
    break;

  case 110:
#line 385 "ffscript.ypp"
    {ASTFuncCall *fc = (ASTFuncCall *)(yyvsp[-1]);
							    ASTExpr *name = (ASTExpr *)(yyvsp[-3]);
							    fc->setName(name);
							    (yyval)=fc;;}
    break;

  case 111:
#line 389 "ffscript.ypp"
    {ASTFuncCall *fc = new ASTFuncCall((yylsp[-2]));
					    ASTExpr *name = (ASTExpr *)(yyvsp[-2]);
					    fc->setName(name);
					    (yyval)=fc;;}
    break;

  case 112:
#line 395 "ffscript.ypp"
    {ASTFuncCall *fc = (ASTFuncCall *)(yyvsp[0]);
					  ASTExpr *e = (ASTExpr *)(yyvsp[-2]);
					  fc->addParam(e);
					  (yyval) = fc;;}
    break;

  case 113:
#line 399 "ffscript.ypp"
    {ASTFuncCall *fc = new ASTFuncCall((yylsp[0]));
		  ASTExpr *e = (ASTExpr *)(yyvsp[0]);
		  fc->addParam(e);
		  (yyval) = fc;;}
    break;

  case 114:
#line 405 "ffscript.ypp"
    {ASTStmt *prec = (ASTStmt *)(yyvsp[-6]);
												ASTExpr *term = (ASTExpr *)(yyvsp[-4]);
												ASTStmt *incr = (ASTExpr *)(yyvsp[-2]);
												ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
												(yyval) = new ASTStmtFor(prec,term,incr,stmt,(yylsp[-8]));;}
    break;

  case 115:
#line 412 "ffscript.ypp"
    {ASTExpr *cond = (ASTExpr *)(yyvsp[-2]);
										   ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
										   (yyval) = new ASTStmtWhile(cond,stmt,(yylsp[-4]));;}
    break;

  case 116:
#line 416 "ffscript.ypp"
    {ASTExpr *cond = (ASTExpr *)(yyvsp[-2]);
							  ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
							  (yyval) = new ASTStmtIf(cond,stmt,(yylsp[-4]));;}
    break;

  case 117:
#line 419 "ffscript.ypp"
    {ASTExpr *cond = (ASTExpr *)(yyvsp[-4]);
										ASTStmt *ifstmt = (ASTStmt *)(yyvsp[-2]);
										ASTStmt *elsestmt = (ASTStmt *)(yyvsp[0]);
										(yyval) = new ASTStmtIfElse(cond,ifstmt,elsestmt,(yylsp[-6]));;}
    break;

  case 118:
#line 425 "ffscript.ypp"
    {(yyval) = new ASTStmtReturnVal((ASTExpr *)(yyvsp[0]),(yylsp[-1]));;}
    break;

  case 119:
#line 426 "ffscript.ypp"
    {(yyval) = new ASTStmtReturn((yylsp[0]));;}
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 2188 "y.tab.cpp"

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


#line 429 "ffscript.ypp"


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

