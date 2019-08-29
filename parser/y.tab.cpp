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
#line 277 "y.tab.cpp"

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
#define YYLAST   421

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  72
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  46
/* YYNRULES -- Number of rules. */
#define YYNRULES  136
/* YYNRULES -- Number of states. */
#define YYNSTATES  224

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   326

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
      65,    66,    67,    68,    69,    70,    71
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     5,     8,     9,    11,    13,    15,    17,
      24,    29,    31,    33,    36,    40,    43,    46,    48,    51,
      56,    58,    61,    63,    65,    67,    69,    71,    73,    75,
      82,    88,    92,    94,    98,   101,   104,   106,   109,   112,
     115,   117,   119,   121,   124,   126,   128,   130,   133,   136,
     139,   141,   143,   145,   147,   149,   151,   153,   155,   157,
     158,   160,   162,   164,   168,   172,   176,   180,   184,   188,
     192,   196,   200,   204,   208,   212,   216,   218,   220,   224,
     226,   230,   237,   241,   243,   247,   249,   253,   255,   259,
     261,   265,   267,   271,   273,   277,   279,   281,   283,   285,
     287,   289,   291,   293,   295,   299,   301,   303,   305,   309,
     311,   313,   315,   317,   320,   323,   326,   328,   332,   334,
     337,   340,   342,   344,   346,   348,   350,   355,   359,   363,
     365,   375,   381,   388,   394,   402,   405
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      73,     0,    -1,    74,    -1,    74,    75,    -1,    -1,    77,
      -1,    79,    -1,    85,    -1,    76,    -1,    21,     4,    48,
      24,    35,    25,    -1,    78,     3,    48,    80,    -1,    84,
      -1,    18,    -1,    11,    49,    -1,    32,    81,    33,    -1,
      32,    33,    -1,    82,    81,    -1,    82,    -1,    83,    25,
      -1,    83,    24,    95,    25,    -1,    85,    -1,    84,    48,
      -1,     4,    -1,     6,    -1,     7,    -1,    15,    -1,    16,
      -1,    17,    -1,    23,    -1,    84,    48,    29,    86,    30,
      87,    -1,    84,    48,    29,    30,    87,    -1,    83,    26,
      86,    -1,    83,    -1,    32,    88,    33,    -1,    32,    33,
      -1,    88,    89,    -1,    89,    -1,    83,    25,    -1,    92,
      25,    -1,    91,    25,    -1,   113,    -1,   116,    -1,    87,
      -1,   117,    25,    -1,   114,    -1,   115,    -1,    25,    -1,
      95,    25,    -1,    19,    25,    -1,    20,    25,    -1,    83,
      -1,    92,    -1,    91,    -1,   113,    -1,   116,    -1,    87,
      -1,   117,    -1,   114,    -1,   115,    -1,    -1,    95,    -1,
      19,    -1,    20,    -1,    94,    36,    95,    -1,    94,    37,
      95,    -1,    94,    38,    95,    -1,    94,    39,    95,    -1,
      94,    40,    95,    -1,    94,    41,    95,    -1,    94,    42,
      95,    -1,    94,    43,    95,    -1,    94,    44,    95,    -1,
      94,    46,    95,    -1,    94,    47,    95,    -1,    94,    45,
      95,    -1,    93,    24,    95,    -1,    83,    -1,    94,    -1,
      48,    31,    48,    -1,    48,    -1,    94,    34,    48,    -1,
      94,    34,    48,    27,    95,    28,    -1,    95,    55,    96,
      -1,    96,    -1,    96,    56,    97,    -1,    97,    -1,    97,
      53,    98,    -1,    98,    -1,    98,    52,    99,    -1,    99,
      -1,    99,    54,   100,    -1,   100,    -1,   100,   103,   101,
      -1,   101,    -1,   101,   102,   104,    -1,   104,    -1,    51,
      -1,    50,    -1,    63,    -1,    64,    -1,    65,    -1,    66,
      -1,    62,    -1,    61,    -1,   104,   105,   106,    -1,   106,
      -1,    68,    -1,    67,    -1,   106,   107,   108,    -1,   108,
      -1,    70,    -1,    69,    -1,    71,    -1,    58,   108,    -1,
      67,   108,    -1,    57,   108,    -1,   109,    -1,    29,    95,
      30,    -1,    94,    -1,    94,    60,    -1,    94,    59,    -1,
      35,    -1,   110,    -1,   111,    -1,    12,    -1,    13,    -1,
      94,    29,   112,    30,    -1,    94,    29,    30,    -1,    95,
      26,   112,    -1,    95,    -1,     5,    29,    90,    25,    95,
      25,    90,    30,    89,    -1,    14,    29,    95,    30,    89,
      -1,    22,    89,    14,    29,    95,    30,    -1,     8,    29,
      95,    30,    89,    -1,     8,    29,    95,    30,    89,     9,
      89,    -1,    10,    95,    -1,    10,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   103,   103,   106,   109,   112,   113,   114,   115,   118,
     123,   130,   131,   134,   139,   140,   143,   146,   151,   152,
     157,   160,   166,   167,   168,   169,   170,   171,   172,   175,
     184,   195,   198,   203,   204,   207,   211,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   262,   265,   266,   269,   273,
     276,   280,   288,   294,   297,   303,   306,   312,   315,   321,
     324,   330,   333,   339,   342,   348,   351,   352,   355,   356,
     357,   358,   359,   360,   363,   369,   372,   373,   376,   382,
     385,   386,   387,   390,   394,   398,   402,   405,   406,   407,
     411,   415,   417,   418,   421,   422,   425,   429,   435,   439,
     445,   452,   456,   460,   463,   469,   470
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SCRIPT", "FLOAT", "FOR", "BOOL", "VOID",
  "IF", "ELSE", "RETURN", "IMPORT", "TRUE", "FALSE", "WHILE", "FFC",
  "ITEM", "ITEMCLASS", "GLOBAL", "BREAK", "CONTINUE", "CONST", "DO", "NPC",
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
     325,   326
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    72,    73,    74,    74,    75,    75,    75,    75,    76,
      77,    78,    78,    79,    80,    80,    81,    81,    82,    82,
      82,    83,    84,    84,    84,    84,    84,    84,    84,    85,
      85,    86,    86,    87,    87,    88,    88,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    92,    93,    93,    94,    94,
      94,    94,    95,    95,    96,    96,    97,    97,    98,    98,
      99,    99,   100,   100,   101,   101,   102,   102,   103,   103,
     103,   103,   103,   103,   104,   104,   105,   105,   106,   106,
     107,   107,   107,   108,   108,   108,   108,   109,   109,   109,
     109,   109,   109,   109,   110,   110,   111,   111,   112,   112,
     113,   114,   115,   116,   116,   117,   117
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     2,     0,     1,     1,     1,     1,     6,
       4,     1,     1,     2,     3,     2,     2,     1,     2,     4,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     6,
       5,     3,     1,     3,     2,     2,     1,     2,     2,     2,
       1,     1,     1,     2,     1,     1,     1,     2,     2,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     1,     3,     1,
       3,     6,     3,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     1,     1,     3,     1,
       1,     1,     1,     2,     2,     2,     1,     3,     1,     2,
       2,     1,     1,     1,     1,     1,     4,     3,     3,     1,
       9,     5,     6,     5,     7,     2,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       4,     0,     2,     1,    22,    23,    24,     0,    25,    26,
      27,    12,     0,    28,     3,     8,     5,     0,     6,    11,
       7,    13,     0,     0,     0,     0,     0,     0,     0,     0,
      10,     0,    32,     0,     0,     0,    15,     0,    17,     0,
       0,    20,     0,    30,     0,    21,     0,     9,    14,    16,
       0,    18,    21,     0,     0,   136,   124,   125,     0,     0,
       0,     0,    46,     0,    34,   121,    79,     0,     0,     0,
      76,    42,     0,    36,     0,     0,     0,   118,     0,    83,
      85,    87,    89,    91,    93,    95,   105,   109,   116,   122,
     123,    40,    44,    45,    41,     0,    31,    29,   118,     0,
      59,     0,   135,     0,    48,    49,     0,     0,     0,   115,
     113,   114,    37,    33,    35,    39,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   120,   119,    47,     0,     0,     0,     0,     0,
     103,   102,    98,    99,   100,   101,     0,    97,    96,     0,
     107,   106,     0,   111,   110,   112,     0,    43,    19,    61,
      62,    50,    55,     0,    52,    51,    60,    53,    57,    58,
      54,    56,     0,     0,     0,   117,    78,    75,   127,   129,
       0,    80,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    74,    72,    73,    82,    84,    86,    88,    90,    92,
      94,   104,   108,     0,     0,     0,     0,     0,   126,     0,
       0,   133,   131,     0,   128,     0,    59,     0,   132,    81,
       0,   134,     0,   130
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,     1,     2,    14,    15,    16,    17,    18,    30,    37,
      38,    70,    33,    41,    34,    71,    72,    73,   163,    74,
      75,    76,    98,    78,    79,    80,    81,    82,    83,    84,
     149,   146,    85,   152,    86,   156,    87,    88,    89,    90,
     180,    91,    92,    93,    94,    95
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -97
static const short int yypact[] =
{
     -97,    21,   392,   -97,   -97,   -97,   -97,   -23,   -97,   -97,
     -97,   -97,    34,   -97,   -97,   -97,   -97,    28,   -97,    -6,
     -97,   -97,    -4,    20,    51,    52,    73,   266,    80,   125,
     -97,    84,    57,    71,    97,   105,   -97,   100,   222,     4,
      86,   -97,   186,   -97,   222,   -97,    84,   -97,   -97,   -97,
     175,   -97,    51,   106,   108,   175,   -97,   -97,   109,   114,
     118,   298,   -97,   175,   -97,   -97,   113,   175,   175,   175,
     122,   -97,   242,   -97,   124,   127,   129,    12,   -12,    98,
     103,   107,   110,    45,   -16,    46,    27,   -97,   -97,   -97,
     -97,   -97,   -97,   -97,   -97,   132,   -97,   -97,     3,   -10,
     354,   175,   117,   175,   -97,   -97,   146,    44,   121,   -97,
     -97,   -97,   -97,   -97,   -97,   -97,   -97,   175,    88,   126,
     175,   175,   175,   175,   175,   175,   175,   175,   175,   175,
     175,   175,   -97,   -97,   -97,   175,   175,   175,   175,   175,
     -97,   -97,   -97,   -97,   -97,   -97,   175,   -97,   -97,   175,
     -97,   -97,   175,   -97,   -97,   -97,   175,   -97,   -97,   -97,
     -97,   137,   -97,   150,   -97,   -97,   117,   -97,   -97,   -97,
     -97,   -97,    47,    48,   133,   -97,   -97,   117,   -97,    14,
     149,   154,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,    98,   103,   107,   110,    45,   -16,
      46,    27,   -97,   175,   298,   298,   175,   175,   -97,   175,
       5,   167,   -97,    49,   -97,    -8,   354,   298,   -97,   -97,
     152,   -97,   298,   -97
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
     -97,   -97,   -97,   -97,   -97,   -97,   -97,   -97,   -97,   147,
     -97,   -27,    37,   182,   151,   -30,   -97,   -54,   -19,   -96,
     -95,   -97,   -39,   -36,    72,    76,    77,    75,    78,    70,
     -97,   -97,    81,   -97,    68,   -97,   -44,   -97,   -97,   -97,
      15,   -94,   -92,   -91,   -90,   -88
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -78
static const short int yytable[] =
{
      32,    43,    39,    77,   164,   165,   167,   106,   168,   169,
     170,    39,   171,   134,    99,   158,    97,    32,   114,   102,
     219,     3,    77,   109,   110,   111,    21,   107,    50,    51,
     216,    23,   118,    77,   147,   148,   -77,   119,    22,    19,
     207,   118,    24,   135,    25,   135,   119,   135,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     135,    77,   132,   133,   166,   172,    40,   173,    26,   135,
     162,   132,   133,   161,   175,    40,    28,   204,   205,   218,
      27,   177,   179,    44,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   153,   154,   155,   135,
      56,    57,   135,   135,   135,    29,   140,   141,   142,   143,
     144,   145,   202,   150,   151,    35,    42,    63,   178,    45,
     164,   165,   167,    65,   168,   169,   170,    46,   171,     4,
      47,     5,     6,    48,    52,   100,    66,   101,   103,   104,
       8,     9,    10,   105,   108,    67,    68,   112,    13,   115,
     211,   212,   116,   117,   136,    69,   137,   157,    36,   138,
     174,   -76,   206,   221,   139,    77,    77,   210,   223,   176,
     213,   179,   135,   215,   181,   203,   217,    77,    77,   208,
     166,   209,   222,    77,    20,    49,   162,    56,    57,   161,
       4,    53,     5,     6,    54,    96,    55,   220,    56,    57,
      58,     8,     9,    10,    63,    59,    60,   194,    61,    13,
      65,    62,   195,   197,   196,    63,   199,   198,    42,    64,
     201,    65,   214,    66,     0,     0,     4,     0,     5,     6,
     200,     0,    67,    68,    66,     0,     0,     8,     9,    10,
       0,     0,    69,    67,    68,    13,     4,    53,     5,     6,
      54,     0,    55,    69,    56,    57,    58,     8,     9,    10,
       0,    59,    60,     0,    61,    13,     0,    62,     0,     0,
       4,    63,     5,     6,    42,   113,     0,    65,     0,     0,
       0,     8,     9,    10,     0,     0,     0,     0,     0,    13,
      66,     0,     0,     0,     0,     0,    31,     0,     0,    67,
      68,     0,     4,    53,     5,     6,    54,     0,    55,    69,
      56,    57,    58,     8,     9,    10,     0,    59,    60,     0,
      61,    13,     0,    62,     0,     0,     0,    63,     0,     0,
      42,     0,     0,    65,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    66,     0,     0,     0,
       0,     0,     0,     0,     0,    67,    68,     0,     4,    53,
       5,     6,    54,     0,    55,    69,    56,    57,    58,     8,
       9,    10,     0,   159,   160,     0,    61,    13,     0,     0,
       0,     0,     0,    63,     0,     0,    42,     0,     0,    65,
       0,     0,     0,     0,     0,     0,     4,     0,     5,     6,
       0,     0,    66,     7,     0,     0,     0,     8,     9,    10,
      11,    67,    68,    12,     0,    13,     0,     0,     0,     0,
       0,    69
};

static const short int yycheck[] =
{
      27,    31,    29,    42,   100,   100,   100,    61,   100,   100,
     100,    38,   100,    25,    50,    25,    46,    44,    72,    55,
      28,     0,    61,    67,    68,    69,    49,    63,    24,    25,
      25,     3,    29,    72,    50,    51,    24,    34,     4,     2,
      26,    29,    48,    55,    48,    55,    34,    55,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      55,   100,    59,    60,   100,   101,    29,   103,    48,    55,
     100,    59,    60,   100,    30,    38,    24,    30,    30,    30,
      29,   117,   118,    26,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,    69,    70,    71,    55,
      12,    13,    55,    55,    55,    32,    61,    62,    63,    64,
      65,    66,   156,    67,    68,    35,    32,    29,    30,    48,
     216,   216,   216,    35,   216,   216,   216,    30,   216,     4,
      25,     6,     7,    33,    48,    29,    48,    29,    29,    25,
      15,    16,    17,    25,    31,    57,    58,    25,    23,    25,
     204,   205,    25,    24,    56,    67,    53,    25,    33,    52,
      14,    24,    29,   217,    54,   204,   205,   203,   222,    48,
     206,   207,    55,   209,    48,    25,     9,   216,   217,    30,
     216,    27,    30,   222,     2,    38,   216,    12,    13,   216,
       4,     5,     6,     7,     8,    44,    10,   216,    12,    13,
      14,    15,    16,    17,    29,    19,    20,   135,    22,    23,
      35,    25,   136,   138,   137,    29,   146,   139,    32,    33,
     152,    35,   207,    48,    -1,    -1,     4,    -1,     6,     7,
     149,    -1,    57,    58,    48,    -1,    -1,    15,    16,    17,
      -1,    -1,    67,    57,    58,    23,     4,     5,     6,     7,
       8,    -1,    10,    67,    12,    13,    14,    15,    16,    17,
      -1,    19,    20,    -1,    22,    23,    -1,    25,    -1,    -1,
       4,    29,     6,     7,    32,    33,    -1,    35,    -1,    -1,
      -1,    15,    16,    17,    -1,    -1,    -1,    -1,    -1,    23,
      48,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    57,
      58,    -1,     4,     5,     6,     7,     8,    -1,    10,    67,
      12,    13,    14,    15,    16,    17,    -1,    19,    20,    -1,
      22,    23,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,
      32,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    58,    -1,     4,     5,
       6,     7,     8,    -1,    10,    67,    12,    13,    14,    15,
      16,    17,    -1,    19,    20,    -1,    22,    23,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    -1,    32,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,     6,     7,
      -1,    -1,    48,    11,    -1,    -1,    -1,    15,    16,    17,
      18,    57,    58,    21,    -1,    23,    -1,    -1,    -1,    -1,
      -1,    67
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    73,    74,     0,     4,     6,     7,    11,    15,    16,
      17,    18,    21,    23,    75,    76,    77,    78,    79,    84,
      85,    49,     4,     3,    48,    48,    48,    29,    24,    32,
      80,    30,    83,    84,    86,    35,    33,    81,    82,    83,
      84,    85,    32,    87,    26,    48,    30,    25,    33,    81,
      24,    25,    48,     5,     8,    10,    12,    13,    14,    19,
      20,    22,    25,    29,    33,    35,    48,    57,    58,    67,
      83,    87,    88,    89,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   104,   106,   108,   109,   110,
     111,   113,   114,   115,   116,   117,    86,    87,    94,    95,
      29,    29,    95,    29,    25,    25,    89,    95,    31,   108,
     108,   108,    25,    33,    89,    25,    25,    24,    29,    34,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    59,    60,    25,    55,    56,    53,    52,    54,
      61,    62,    63,    64,    65,    66,   103,    50,    51,   102,
      67,    68,   105,    69,    70,    71,   107,    25,    25,    19,
      20,    83,    87,    90,    91,    92,    95,   113,   114,   115,
     116,   117,    95,    95,    14,    30,    48,    95,    30,    95,
     112,    48,    95,    95,    95,    95,    95,    95,    95,    95,
      95,    95,    95,    95,    96,    97,    98,    99,   100,   101,
     104,   106,   108,    25,    30,    30,    29,    26,    30,    27,
      95,    89,    89,    95,   112,    95,    25,     9,    30,    28,
      90,    89,    30,    89
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
#line 103 "ffscript.ypp"
    {resAST = new ASTProgram((ASTDeclList *)(yyvsp[0]), (yylsp[0]));;}
    break;

  case 3:
#line 106 "ffscript.ypp"
    {ASTDeclList *list = (ASTDeclList *)(yyvsp[-1]);
				list->addDeclaration((ASTDecl *)(yyvsp[0]));
				(yyval) = list;;}
    break;

  case 4:
#line 109 "ffscript.ypp"
    {(yyval) = new ASTDeclList(noloc);;}
    break;

  case 5:
#line 112 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 6:
#line 113 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 7:
#line 114 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 8:
#line 115 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 9:
#line 118 "ffscript.ypp"
    {ASTString *name = (ASTString *)(yyvsp[-3]);
															ASTFloat *val = (ASTFloat *)(yyvsp[-1]);
															(yyval) = new ASTConstDecl(name->getValue(), val,(yylsp[-5]));
															delete name;;}
    break;

  case 10:
#line 123 "ffscript.ypp"
    {ASTType *type = (ASTType *)(yyvsp[-3]);
								   ASTString *name = (ASTString *)(yyvsp[-1]);
								   ASTDeclList *sblock = (ASTDeclList *)(yyvsp[0]);
								   (yyval) = new ASTScript(type, name->getValue(), sblock,(yylsp[-3]));
								   delete name;;}
    break;

  case 11:
#line 130 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 12:
#line 131 "ffscript.ypp"
    {(yyval) = new ASTTypeGlobal((yylsp[0]));;}
    break;

  case 13:
#line 134 "ffscript.ypp"
    {ASTString *str = (ASTString *)(yyvsp[0]);
					 (yyval) = new ASTImportDecl(str->getValue(),(yylsp[-1]));
					 delete str;;}
    break;

  case 14:
#line 139 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 15:
#line 140 "ffscript.ypp"
    {(yyval) = new ASTDeclList((yylsp[-1]));;}
    break;

  case 16:
#line 143 "ffscript.ypp"
    {ASTDeclList *dl = (ASTDeclList *)(yyvsp[0]);
							  dl->addDeclaration((ASTDecl *)(yyvsp[-1]));
							  (yyval) = dl;;}
    break;

  case 17:
#line 146 "ffscript.ypp"
    {ASTDeclList *dl = new ASTDeclList((yylsp[0]));
			  dl->addDeclaration((ASTDecl *)(yyvsp[0]));
			  (yyval) = dl;;}
    break;

  case 18:
#line 151 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 19:
#line 152 "ffscript.ypp"
    {ASTVarDecl *vd = (ASTVarDecl *)(yyvsp[-3]);
						   Clone c;
						   vd->getType()->execute(c,NULL);
						   (yyval) = new ASTVarDeclInitializer((ASTType *)c.getResult(), vd->getName(), (ASTExpr *)(yyvsp[-1]),(yylsp[-3]));
						   delete vd;;}
    break;

  case 20:
#line 157 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 21:
#line 160 "ffscript.ypp"
    {ASTType *type = (ASTType *)(yyvsp[-1]);
				   ASTString *name = (ASTString *)(yyvsp[0]);
				   (yyval) = new ASTVarDecl(type, name->getValue(),(yylsp[-1]));
				   delete name;;}
    break;

  case 22:
#line 166 "ffscript.ypp"
    {(yyval) = new ASTTypeFloat((yylsp[0]));;}
    break;

  case 23:
#line 167 "ffscript.ypp"
    {(yyval) = new ASTTypeBool((yylsp[0]));;}
    break;

  case 24:
#line 168 "ffscript.ypp"
    {(yyval) = new ASTTypeVoid((yylsp[0]));;}
    break;

  case 25:
#line 169 "ffscript.ypp"
    {(yyval) = new ASTTypeFFC((yylsp[0]));;}
    break;

  case 26:
#line 170 "ffscript.ypp"
    {(yyval) = new ASTTypeItem((yylsp[0]));;}
    break;

  case 27:
#line 171 "ffscript.ypp"
    {(yyval) = new ASTTypeItemclass((yylsp[0]));;}
    break;

  case 28:
#line 172 "ffscript.ypp"
    {(yyval) = new ASTTypeNPC((yylsp[0]));;}
    break;

  case 29:
#line 175 "ffscript.ypp"
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
#line 184 "ffscript.ypp"
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
#line 195 "ffscript.ypp"
    {ASTFuncDecl *fd = (ASTFuncDecl *)(yyvsp[0]);
						 fd->addParam((ASTVarDecl *)(yyvsp[-2]));
						 (yyval)=fd;;}
    break;

  case 32:
#line 198 "ffscript.ypp"
    {ASTFuncDecl *fd = new ASTFuncDecl((yylsp[0]));
		     fd->addParam((ASTVarDecl *)(yyvsp[0]));
		     (yyval)=fd;;}
    break;

  case 33:
#line 203 "ffscript.ypp"
    {(yyval)=(yyvsp[-1]);;}
    break;

  case 34:
#line 204 "ffscript.ypp"
    {(yyval) = new ASTBlock((yylsp[-1]));;}
    break;

  case 35:
#line 207 "ffscript.ypp"
    {ASTBlock *block = (ASTBlock *)(yyvsp[-1]);
				  ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
				  block->addStatement(stmt);
				  (yyval) = block;;}
    break;

  case 36:
#line 211 "ffscript.ypp"
    {ASTBlock *block = new ASTBlock((yylsp[0]));
		  ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
		  block->addStatement(stmt);
		  (yyval) = block;;}
    break;

  case 37:
#line 217 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 38:
#line 218 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 39:
#line 219 "ffscript.ypp"
    {(yyval)=(yyvsp[-1]);;}
    break;

  case 40:
#line 220 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 41:
#line 221 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 42:
#line 222 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 43:
#line 223 "ffscript.ypp"
    {(yyval) = (yyvsp[-1]);;}
    break;

  case 44:
#line 224 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 45:
#line 225 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 46:
#line 226 "ffscript.ypp"
    {(yyval) = new ASTStmtEmpty((yylsp[0]));;}
    break;

  case 47:
#line 227 "ffscript.ypp"
    {(yyval)=(yyvsp[-1]);;}
    break;

  case 48:
#line 228 "ffscript.ypp"
    {(yyval) = new ASTStmtBreak((yylsp[-1]));;}
    break;

  case 49:
#line 229 "ffscript.ypp"
    {(yyval) = new ASTStmtContinue((yylsp[-1]));;}
    break;

  case 50:
#line 232 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 51:
#line 233 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 52:
#line 234 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 53:
#line 235 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 54:
#line 236 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 55:
#line 237 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 56:
#line 238 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 57:
#line 239 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 58:
#line 240 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 59:
#line 241 "ffscript.ypp"
    {(yyval) = new ASTStmtEmpty(noloc);;}
    break;

  case 60:
#line 242 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 61:
#line 243 "ffscript.ypp"
    {(yyval) = new ASTStmtBreak((yylsp[0]));;}
    break;

  case 62:
#line 244 "ffscript.ypp"
    {(yyval) = new ASTStmtContinue((yylsp[0]));;}
    break;

  case 63:
#line 247 "ffscript.ypp"
    {SHORTCUT(ASTExprPlus,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 64:
#line 248 "ffscript.ypp"
    {SHORTCUT(ASTExprMinus,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 65:
#line 249 "ffscript.ypp"
    {SHORTCUT(ASTExprTimes,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 66:
#line 250 "ffscript.ypp"
    {SHORTCUT(ASTExprDivide,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 67:
#line 251 "ffscript.ypp"
    {SHORTCUT(ASTExprAnd,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 68:
#line 252 "ffscript.ypp"
    {SHORTCUT(ASTExprOr,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 69:
#line 253 "ffscript.ypp"
    {SHORTCUT(ASTExprBitAnd,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 70:
#line 254 "ffscript.ypp"
    {SHORTCUT(ASTExprBitOr,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 71:
#line 255 "ffscript.ypp"
    {SHORTCUT(ASTExprBitXor,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 72:
#line 256 "ffscript.ypp"
    {SHORTCUT(ASTExprLShift,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 73:
#line 257 "ffscript.ypp"
    {SHORTCUT(ASTExprRShift,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 74:
#line 258 "ffscript.ypp"
    {SHORTCUT(ASTExprModulo,(yyvsp[-2]),(yyvsp[0]),(yyval),(yylsp[-2]),(yylsp[-1])) ;}
    break;

  case 75:
#line 262 "ffscript.ypp"
    {(yyval) = new ASTStmtAssign((ASTStmt *)(yyvsp[-2]), (ASTExpr *)(yyvsp[0]),(yylsp[-2]));;}
    break;

  case 76:
#line 265 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 77:
#line 266 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 78:
#line 269 "ffscript.ypp"
    {ASTString *lval = (ASTString *)(yyvsp[-2]);
						 ASTString *rval = (ASTString *)(yyvsp[0]);
						 (yyval) = new ASTExprDot(lval->getValue(), rval->getValue(),(yylsp[-2]));
						 delete lval; delete rval;;}
    break;

  case 79:
#line 273 "ffscript.ypp"
    {ASTString *rval = (ASTString *)(yyvsp[0]);
				  (yyval) = new ASTExprDot("", rval->getValue(),(yylsp[0]));
				  delete rval;;}
    break;

  case 80:
#line 276 "ffscript.ypp"
    {ASTExpr *id = (ASTExpr *)(yyvsp[-2]);
								ASTString *rval = (ASTString *)(yyvsp[0]);
								(yyval) = new ASTExprArrow(id, rval->getValue(), (yylsp[-2]));
								delete rval;;}
    break;

  case 81:
#line 280 "ffscript.ypp"
    {ASTExpr *id = (ASTExpr *)(yyvsp[-5]);
											       	   ASTString *rval = (ASTString *)(yyvsp[-3]);
													   ASTExpr *num = (ASTExpr *)(yyvsp[-1]);
													   ASTExprArrow *res = new ASTExprArrow(id, rval->getValue(), (yylsp[-5]));
													   res->setIndex(num);
													   (yyval) = res;;}
    break;

  case 82:
#line 288 "ffscript.ypp"
    {ASTLogExpr *e = new ASTExprOr((yylsp[-1]));
				 ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				 ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				 e->setFirstOperand(left);
				 e->setSecondOperand(right);
				 (yyval)=e;;}
    break;

  case 83:
#line 294 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 84:
#line 297 "ffscript.ypp"
    {ASTLogExpr *e = new ASTExprAnd((yylsp[-1]));
				 ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				 ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				 e->setFirstOperand(left);
				 e->setSecondOperand(right);
				 (yyval)=e;;}
    break;

  case 85:
#line 303 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 86:
#line 306 "ffscript.ypp"
    {ASTBitExpr *e = new ASTExprBitOr((yylsp[-1]));
							  ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							  ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							  e->setFirstOperand(left);
							  e->setSecondOperand(right);
							  (yyval)=e;;}
    break;

  case 87:
#line 312 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 88:
#line 315 "ffscript.ypp"
    {ASTBitExpr *e = new ASTExprBitXor((yylsp[-1]));
							   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							   e->setFirstOperand(left);
							   e->setSecondOperand(right);
							   (yyval)=e;;}
    break;

  case 89:
#line 321 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 90:
#line 324 "ffscript.ypp"
    {ASTBitExpr *e = new ASTExprBitAnd((yylsp[-1]));
							   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							   e->setFirstOperand(left);
							   e->setSecondOperand(right);
							   (yyval)=e;;}
    break;

  case 91:
#line 330 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 92:
#line 333 "ffscript.ypp"
    {ASTRelExpr *e = (ASTRelExpr *)(yyvsp[-1]);
				   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				   e->setFirstOperand(left);
				   e->setSecondOperand(right);
				   (yyval)=e;;}
    break;

  case 93:
#line 339 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 94:
#line 342 "ffscript.ypp"
    {ASTShiftExpr *e = (ASTShiftExpr *)(yyvsp[-1]);
							   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
							   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
							   e->setFirstOperand(left);
							   e->setSecondOperand(right);
							   (yyval)=e;;}
    break;

  case 95:
#line 348 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 96:
#line 351 "ffscript.ypp"
    {(yyval) = new ASTExprLShift((yylsp[0]));;}
    break;

  case 97:
#line 352 "ffscript.ypp"
    {(yyval) = new ASTExprRShift((yylsp[0]));;}
    break;

  case 98:
#line 355 "ffscript.ypp"
    {(yyval) = new ASTExprGT((yylsp[0]));;}
    break;

  case 99:
#line 356 "ffscript.ypp"
    {(yyval) = new ASTExprGE((yylsp[0]));;}
    break;

  case 100:
#line 357 "ffscript.ypp"
    {(yyval) = new ASTExprLT((yylsp[0]));;}
    break;

  case 101:
#line 358 "ffscript.ypp"
    {(yyval) = new ASTExprLE((yylsp[0]));;}
    break;

  case 102:
#line 359 "ffscript.ypp"
    {(yyval) = new ASTExprEQ((yylsp[0]));;}
    break;

  case 103:
#line 360 "ffscript.ypp"
    {(yyval) = new ASTExprNE((yylsp[0]));;}
    break;

  case 104:
#line 363 "ffscript.ypp"
    {ASTAddExpr *e = (ASTAddExpr *)(yyvsp[-1]);
				   ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				   ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				   e->setFirstOperand(left);
				   e->setSecondOperand(right);
				   (yyval)=e;;}
    break;

  case 105:
#line 369 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 106:
#line 372 "ffscript.ypp"
    {(yyval) = new ASTExprPlus((yylsp[0]));;}
    break;

  case 107:
#line 373 "ffscript.ypp"
    {(yyval) = new ASTExprMinus((yylsp[0]));;}
    break;

  case 108:
#line 376 "ffscript.ypp"
    {ASTMultExpr *e = (ASTMultExpr *)(yyvsp[-1]);
				    ASTExpr *left = (ASTExpr *)(yyvsp[-2]);
				    ASTExpr *right = (ASTExpr *)(yyvsp[0]);
				    e->setFirstOperand(left);
				    e->setSecondOperand(right);
				    (yyval)=e;;}
    break;

  case 109:
#line 382 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 110:
#line 385 "ffscript.ypp"
    {(yyval) = new ASTExprTimes((yylsp[0]));;}
    break;

  case 111:
#line 386 "ffscript.ypp"
    {(yyval) = new ASTExprDivide((yylsp[0]));;}
    break;

  case 112:
#line 387 "ffscript.ypp"
    {(yyval) = new ASTExprModulo((yylsp[0]));;}
    break;

  case 113:
#line 390 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprNot((yylsp[-1]));
			 ASTExpr *op = (ASTExpr *)(yyvsp[0]);
			 e->setOperand(op);
			 (yyval)=e;;}
    break;

  case 114:
#line 394 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprNegate((yylsp[-1]));
			   ASTExpr *op = (ASTExpr *)(yyvsp[0]);
			   e->setOperand(op);
			   (yyval)=e;;}
    break;

  case 115:
#line 398 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprBitNot((yylsp[-1]));
					ASTExpr *op = (ASTExpr *)(yyvsp[0]);
					e->setOperand(op);
					(yyval)=e;;}
    break;

  case 116:
#line 402 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 117:
#line 405 "ffscript.ypp"
    {(yyval)=(yyvsp[-1]);;}
    break;

  case 118:
#line 406 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 119:
#line 407 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprIncrement((yylsp[0]));
						 ASTExpr *op = (ASTExpr *)(yyvsp[-1]);
						 e->setOperand(op);
						 (yyval)=e;;}
    break;

  case 120:
#line 411 "ffscript.ypp"
    {ASTUnaryExpr *e = new ASTExprDecrement((yylsp[0]));
						 ASTExpr *op = (ASTExpr *)(yyvsp[-1]);
						 e->setOperand(op);
						 (yyval)=e;;}
    break;

  case 121:
#line 415 "ffscript.ypp"
    {ASTFloat *val = (ASTFloat *)(yyvsp[0]);
		    (yyval) = new ASTNumConstant(val,(yylsp[0]));;}
    break;

  case 122:
#line 417 "ffscript.ypp"
    {(yyval) = (yyvsp[0]);;}
    break;

  case 123:
#line 418 "ffscript.ypp"
    {(yyval)=(yyvsp[0]);;}
    break;

  case 124:
#line 421 "ffscript.ypp"
    {(yyval) = new ASTBoolConstant(true,(yylsp[0]));;}
    break;

  case 125:
#line 422 "ffscript.ypp"
    {(yyval) = new ASTBoolConstant(false,(yylsp[0]));;}
    break;

  case 126:
#line 425 "ffscript.ypp"
    {ASTFuncCall *fc = (ASTFuncCall *)(yyvsp[-1]);
							    ASTExpr *name = (ASTExpr *)(yyvsp[-3]);
							    fc->setName(name);
							    (yyval)=fc;;}
    break;

  case 127:
#line 429 "ffscript.ypp"
    {ASTFuncCall *fc = new ASTFuncCall((yylsp[-2]));
					    ASTExpr *name = (ASTExpr *)(yyvsp[-2]);
					    fc->setName(name);
					    (yyval)=fc;;}
    break;

  case 128:
#line 435 "ffscript.ypp"
    {ASTFuncCall *fc = (ASTFuncCall *)(yyvsp[0]);
					  ASTExpr *e = (ASTExpr *)(yyvsp[-2]);
					  fc->addParam(e);
					  (yyval) = fc;;}
    break;

  case 129:
#line 439 "ffscript.ypp"
    {ASTFuncCall *fc = new ASTFuncCall((yylsp[0]));
		  ASTExpr *e = (ASTExpr *)(yyvsp[0]);
		  fc->addParam(e);
		  (yyval) = fc;;}
    break;

  case 130:
#line 445 "ffscript.ypp"
    {ASTStmt *prec = (ASTStmt *)(yyvsp[-6]);
												ASTExpr *term = (ASTExpr *)(yyvsp[-4]);
												ASTStmt *incr = (ASTExpr *)(yyvsp[-2]);
												ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
												(yyval) = new ASTStmtFor(prec,term,incr,stmt,(yylsp[-8]));;}
    break;

  case 131:
#line 452 "ffscript.ypp"
    {ASTExpr *cond = (ASTExpr *)(yyvsp[-2]);
										   ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
										   (yyval) = new ASTStmtWhile(cond,stmt,(yylsp[-4]));;}
    break;

  case 132:
#line 456 "ffscript.ypp"
    {ASTExpr *cond = (ASTExpr *)(yyvsp[-1]);
										   ASTStmt *stmt = (ASTStmt *)(yyvsp[-4]);
										   (yyval) = new ASTStmtDo(cond,stmt,(yylsp[-5]));;}
    break;

  case 133:
#line 460 "ffscript.ypp"
    {ASTExpr *cond = (ASTExpr *)(yyvsp[-2]);
							  ASTStmt *stmt = (ASTStmt *)(yyvsp[0]);
							  (yyval) = new ASTStmtIf(cond,stmt,(yylsp[-4]));;}
    break;

  case 134:
#line 463 "ffscript.ypp"
    {ASTExpr *cond = (ASTExpr *)(yyvsp[-4]);
										ASTStmt *ifstmt = (ASTStmt *)(yyvsp[-2]);
										ASTStmt *elsestmt = (ASTStmt *)(yyvsp[0]);
										(yyval) = new ASTStmtIfElse(cond,ifstmt,elsestmt,(yylsp[-6]));;}
    break;

  case 135:
#line 469 "ffscript.ypp"
    {(yyval) = new ASTStmtReturnVal((ASTExpr *)(yyvsp[0]),(yylsp[-1]));;}
    break;

  case 136:
#line 470 "ffscript.ypp"
    {(yyval) = new ASTStmtReturn((yylsp[0]));;}
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 2375 "y.tab.cpp"

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


#line 473 "ffscript.ypp"


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

