/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 10 "ffscript.ypp"

#include "../precompiled.h" //always first

#include <stdlib.h>
#include <stdio.h>
#include <cassert>
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


/* Line 189 of yacc.c  */
#line 102 "y.tab.cpp"

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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
/* Put the tokens into the symbol table, so that GDB and other debuggers
   know about them.  */
enum yytokentype
{
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
    MODULO = 329,
    COMMENTBLOCK = 330
};
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

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


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 231 "y.tab.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID(int yyi)
#else
static int
YYID(yyi)
int yyi;
#endif
{
    return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
/* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
/* The OS might guarantee only one guard page at the bottom of the stack,
   and a page size can be as small as 4096 bytes.  So we cannot safely
   invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
   to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc(YYSIZE_T);  /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free(void *);  /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
    yytype_int16 yyss_alloc;
    YYSTYPE yyvs_alloc;
    YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
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
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   549

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  75
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  48
/* YYNRULES -- Number of rules.  */
#define YYNRULES  157
/* YYNRULES -- Number of states.  */
#define YYNSTATES  270

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   329

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
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
    65,    66,    67,    68,    69,    70,    71,    72,    73,    74
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
    0,     0,     3,     5,     8,     9,    11,    13,    15,    17,
    20,    25,    28,    35,    43,    49,    59,    68,    76,    83,
    87,    89,    94,    96,    98,   101,   105,   108,   111,   113,
    116,   119,   124,   126,   129,   131,   133,   135,   137,   139,
    141,   143,   145,   147,   154,   160,   164,   166,   170,   173,
    176,   178,   181,   184,   187,   190,   192,   194,   196,   199,
    201,   203,   205,   208,   211,   214,   216,   218,   220,   222,
    224,   226,   228,   230,   232,   234,   235,   237,   239,   241,
    245,   249,   253,   257,   261,   265,   269,   273,   277,   281,
    285,   289,   293,   295,   297,   301,   306,   313,   315,   319,
    326,   330,   332,   336,   338,   342,   344,   348,   350,   354,
    356,   360,   362,   366,   368,   370,   372,   374,   376,   378,
    380,   382,   384,   388,   390,   392,   394,   398,   400,   402,
    404,   406,   409,   412,   415,   417,   421,   423,   426,   429,
    432,   435,   437,   439,   441,   443,   445,   447,   452,   456,
    460,   462,   472,   478,   485,   491,   499,   502
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
    76,     0,    -1,    77,    -1,    77,    78,    -1,    -1,    82,
    -1,    84,    -1,    90,    -1,    79,    -1,    88,    27,    -1,
    88,    26,   100,    27,    -1,    80,    27,    -1,    21,     4,
    50,    26,    37,    27,    -1,    21,     4,    50,    26,    70,
    37,    27,    -1,    89,    50,    29,    37,    30,    -1,    89,
    50,    29,    37,    30,    26,    34,    81,    35,    -1,    89,
    50,    29,    30,    26,    34,    81,    35,    -1,    89,    50,
    29,    37,    30,    26,    51,    -1,    89,    50,    29,    30,
    26,    51,    -1,    81,    28,   100,    -1,   100,    -1,    83,
    3,    50,    85,    -1,    89,    -1,    18,    -1,    11,    51,
    -1,    34,    86,    35,    -1,    34,    35,    -1,    87,    86,
    -1,    87,    -1,    88,    27,    -1,    80,    27,    -1,    88,
    26,   100,    27,    -1,    90,    -1,    89,    50,    -1,     4,
    -1,     6,    -1,     7,    -1,    15,    -1,    16,    -1,    17,
    -1,    23,    -1,    24,    -1,    25,    -1,    89,    50,    31,
    91,    32,    92,    -1,    89,    50,    31,    32,    92,    -1,
    88,    28,    91,    -1,    88,    -1,    34,    93,    35,    -1,
    34,    35,    -1,    93,    94,    -1,    94,    -1,    88,    27,
    -1,    80,    27,    -1,    97,    27,    -1,    96,    27,    -1,
    118,    -1,   121,    -1,    92,    -1,   122,    27,    -1,   119,
    -1,   120,    -1,    27,    -1,   100,    27,    -1,    19,    27,
    -1,    20,    27,    -1,    88,    -1,    80,    -1,    97,    -1,
    96,    -1,   118,    -1,   121,    -1,    92,    -1,   122,    -1,
    119,    -1,   120,    -1,    -1,   100,    -1,    19,    -1,    20,
    -1,    99,    38,   100,    -1,    99,    39,   100,    -1,    99,
    40,   100,    -1,    99,    41,   100,    -1,    99,    42,   100,
    -1,    99,    43,   100,    -1,    99,    44,   100,    -1,    99,
    45,   100,    -1,    99,    46,   100,    -1,    99,    48,   100,
    -1,    99,    49,   100,    -1,    99,    47,   100,    -1,    98,
    26,   100,    -1,    88,    -1,    99,    -1,    50,    33,    50,
    -1,    50,    29,   100,    30,    -1,    50,    33,    50,    29,
    100,    30,    -1,    50,    -1,    99,    36,    50,    -1,    99,
    36,    50,    29,   100,    30,    -1,   100,    58,   101,    -1,
    101,    -1,   101,    59,   102,    -1,   102,    -1,   102,    56,
    103,    -1,   103,    -1,   103,    55,   104,    -1,   104,    -1,
    104,    57,   105,    -1,   105,    -1,   105,   108,   106,    -1,
    106,    -1,   106,   107,   109,    -1,   109,    -1,    54,    -1,
    53,    -1,    66,    -1,    67,    -1,    68,    -1,    69,    -1,
    65,    -1,    64,    -1,   109,   110,   111,    -1,   111,    -1,
    71,    -1,    70,    -1,   111,   112,   113,    -1,   113,    -1,
    73,    -1,    72,    -1,    74,    -1,    61,   113,    -1,    70,
    113,    -1,    60,   113,    -1,   114,    -1,    31,   100,    32,
    -1,    99,    -1,    99,    63,    -1,    63,    99,    -1,    99,
    62,    -1,    62,    99,    -1,    37,    -1,    52,    -1,   115,
    -1,   116,    -1,    12,    -1,    13,    -1,    99,    31,   117,
    32,    -1,    99,    31,    32,    -1,   100,    28,   117,    -1,
    100,    -1,     5,    31,    95,    27,   100,    27,    95,    32,
    94,    -1,    14,    31,   100,    32,    94,    -1,    22,    94,
    14,    31,   100,    32,    -1,     8,    31,   100,    32,    94,
    -1,     8,    31,   100,    32,    94,     9,    94,    -1,    10,
    100,    -1,    10,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
    0,   107,   107,   110,   113,   116,   117,   118,   119,   120,
    121,   126,   129,   133,   139,   145,   151,   160,   174,   191,
    194,   199,   215,   216,   219,   224,   225,   228,   231,   236,
    237,   238,   243,   246,   252,   253,   254,   255,   256,   257,
    258,   259,   260,   263,   272,   283,   286,   291,   292,   295,
    299,   305,   306,   307,   308,   309,   310,   311,   312,   313,
    314,   315,   316,   317,   318,   321,   322,   323,   324,   325,
    326,   327,   328,   329,   330,   331,   332,   333,   334,   337,
    338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
    348,   352,   355,   356,   359,   363,   369,   377,   380,   384,
    392,   398,   401,   407,   410,   416,   419,   425,   428,   434,
    437,   443,   446,   452,   455,   456,   459,   460,   461,   462,
    463,   464,   467,   473,   476,   477,   480,   486,   489,   490,
    491,   494,   498,   502,   506,   509,   510,   511,   515,   519,
    523,   527,   529,   533,   534,   537,   538,   541,   545,   551,
    555,   561,   568,   572,   576,   579,   585,   586
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
    "$end", "error", "$undefined", "SCRIPT", "FLOAT", "FOR", "BOOL", "VOID",
    "IF", "ELSE", "RETURN", "IMPORT", "TRUE", "FALSE", "WHILE", "FFC",
    "ITEM", "ITEMCLASS", "GLOBAL", "BREAK", "CONTINUE", "CONST", "DO", "NPC",
    "LWEAPON", "EWEAPON", "ASSIGN", "SEMICOLON", "COMMA", "LBRACKET",
    "RBRACKET", "LPAREN", "RPAREN", "DOT", "LBRACE", "RBRACE", "ARROW",
    "NUMBER", "PLUSASSIGN", "MINUSASSIGN", "TIMESASSIGN", "DIVIDEASSIGN",
    "ANDASSIGN", "ORASSIGN", "BITANDASSIGN", "BITORASSIGN", "BITXORASSIGN",
    "MODULOASSIGN", "LSHIFTASSIGN", "RSHIFTASSIGN", "IDENTIFIER",
    "QUOTEDSTRING", "SINGLECHAR", "RSHIFT", "LSHIFT", "BITXOR", "BITOR",
    "BITAND", "OR", "AND", "BITNOT", "NOT", "DECREMENT", "INCREMENT", "NE",
    "EQ", "GT", "GE", "LT", "LE", "MINUS", "PLUS", "DIVIDE", "TIMES",
    "MODULO", "COMMENTBLOCK", "$accept", "Init", "Prog", "GlobalStmt", "ConstDecl",
    "ArrayDecl", "ArrayList", "Script", "ScriptType", "Import",
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
static const yytype_uint16 yytoknum[] =
{
    0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
    265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
    275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
    285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
    295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
    305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
    315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
    325,   326,   327,   328,   329,	330
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
    0,    75,    76,    77,    77,    78,    78,    78,    78,    78,
    78,    78,    79,    79,    80,    80,    80,    80,    80,    81,
    81,    82,    83,    83,    84,    85,    85,    86,    86,    87,
    87,    87,    87,    88,    89,    89,    89,    89,    89,    89,
    89,    89,    89,    90,    90,    91,    91,    92,    92,    93,
    93,    94,    94,    94,    94,    94,    94,    94,    94,    94,
    94,    94,    94,    94,    94,    95,    95,    95,    95,    95,
    95,    95,    95,    95,    95,    95,    95,    95,    95,    96,
    96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
    96,    97,    98,    98,    99,    99,    99,    99,    99,    99,
    100,   100,   101,   101,   102,   102,   103,   103,   104,   104,
    105,   105,   106,   106,   107,   107,   108,   108,   108,   108,
    108,   108,   109,   109,   110,   110,   111,   111,   112,   112,
    112,   113,   113,   113,   113,   114,   114,   114,   114,   114,
    114,   114,   114,   114,   114,   115,   115,   116,   116,   117,
    117,   118,   119,   120,   121,   121,   122,   122
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
    0,     2,     1,     2,     0,     1,     1,     1,     1,     2,
    4,     2,     6,     7,     5,     9,     8,     7,     6,     3,
    1,     4,     1,     1,     2,     3,     2,     2,     1,     2,
    2,     4,     1,     2,     1,     1,     1,     1,     1,     1,
    1,     1,     1,     6,     5,     3,     1,     3,     2,     2,
    1,     2,     2,     2,     2,     1,     1,     1,     2,     1,
    1,     1,     2,     2,     2,     1,     1,     1,     1,     1,
    1,     1,     1,     1,     1,     0,     1,     1,     1,     3,
    3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
    3,     3,     1,     1,     3,     4,     6,     1,     3,     6,
    3,     1,     3,     1,     3,     1,     3,     1,     3,     1,
    3,     1,     3,     1,     1,     1,     1,     1,     1,     1,
    1,     1,     3,     1,     1,     1,     3,     1,     1,     1,
    1,     2,     2,     2,     1,     3,     1,     2,     2,     2,
    2,     1,     1,     1,     1,     1,     1,     4,     3,     3,
    1,     9,     5,     6,     5,     7,     2,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
    4,     0,     2,     1,    34,    35,    36,     0,    37,    38,
    39,    23,     0,    40,    41,    42,     3,     8,     0,     5,
    0,     6,     0,    22,     7,    24,     0,    11,     0,     0,
    9,    33,     0,     0,   145,   146,     0,   141,    97,   142,
    0,     0,     0,     0,     0,   136,     0,   101,   103,   105,
    107,   109,   111,   113,   123,   127,   134,   143,   144,     0,
    0,     0,     0,    21,     0,     0,     0,   133,   131,   140,
    138,   132,     0,     0,   139,   137,    10,     0,     0,     0,
    0,     0,   121,   120,   116,   117,   118,   119,     0,   115,
    114,     0,   125,   124,     0,   129,   128,   130,     0,     0,
    0,     0,    46,     0,     0,     0,     0,    26,     0,     0,
    28,     0,     0,    32,   135,     0,    94,   148,   150,     0,
    98,   100,   102,   104,   106,   108,   110,   112,   122,   126,
    0,    14,     0,    44,     0,    33,     0,    12,     0,    30,
    25,    27,     0,    29,    95,     0,     0,   147,     0,     0,
    18,     0,     0,     0,   157,     0,     0,     0,     0,    61,
    48,     0,    92,     0,    57,     0,    50,     0,     0,     0,
    136,     0,    55,    59,    60,    56,     0,    45,    43,    13,
    0,     0,   149,     0,     0,    20,     0,    17,    75,     0,
    156,     0,    63,    64,     0,    52,    51,    33,    47,    49,
    54,    53,     0,     0,     0,     0,     0,     0,     0,     0,
    0,     0,     0,     0,     0,    62,    58,    31,    96,    99,
    0,    16,     0,    77,    78,    66,    65,    71,     0,    68,
    67,    76,    69,    73,    74,    70,    72,     0,     0,     0,
    91,    79,    80,    81,    82,    83,    84,    85,    86,    87,
    90,    88,    89,    19,    15,     0,     0,     0,     0,     0,
    154,   152,     0,    75,     0,   153,     0,   155,     0,   151
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
    -1,     1,     2,    16,    17,   161,   184,    19,    20,    21,
    63,   109,   110,   162,   163,   113,   104,   164,   165,   166,
    228,   167,   168,   169,    45,   171,    47,    48,    49,    50,
    51,    52,    91,    88,    53,    94,    54,    98,    55,    56,
    57,    58,   119,   172,   173,   174,   175,   176
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -182
static const yytype_int16 yypact[] =
{
    -182,    33,   524,  -182,  -182,  -182,  -182,    -4,  -182,  -182,
    -182,  -182,    18,  -182,  -182,  -182,  -182,  -182,    30,  -182,
    41,  -182,     9,    31,  -182,  -182,    55,  -182,    66,   177,
    -182,    88,    94,    90,  -182,  -182,   177,  -182,    64,  -182,
    177,   177,    78,    78,   177,   -10,    -3,    83,    87,    95,
    96,   166,    73,   -29,    22,  -182,  -182,  -182,  -182,    -5,
    140,   -20,   106,  -182,    40,   177,   104,  -182,  -182,   122,
    122,  -182,    17,   109,  -182,  -182,  -182,   177,   177,   177,
    177,   177,  -182,  -182,  -182,  -182,  -182,  -182,   177,  -182,
    -182,   177,  -182,  -182,   177,  -182,  -182,  -182,   177,   141,
    143,   137,   146,   125,   144,   151,   147,  -182,   154,   148,
    145,   108,    31,  -182,  -182,   -12,   156,  -182,    12,   159,
    163,    83,    87,    95,    96,   166,    73,   -29,    22,  -182,
    11,   162,   259,  -182,   145,  -182,   137,  -182,   179,  -182,
    -182,  -182,   177,  -182,  -182,   177,   177,  -182,   177,   177,
    -182,    56,   176,   178,   177,   181,   183,   186,   381,  -182,
    -182,   188,   191,   169,  -182,   320,  -182,   193,   194,   196,
    475,     1,  -182,  -182,  -182,  -182,   198,  -182,  -182,  -182,
    7,    -7,  -182,    13,     3,   168,   177,  -182,   442,   177,
    168,   177,  -182,  -182,   222,  -182,  -182,   212,  -182,  -182,
    -182,  -182,   177,   177,   177,   177,   177,   177,   177,   177,
    177,   177,   177,   177,   177,  -182,  -182,  -182,  -182,  -182,
    177,  -182,    71,  -182,  -182,  -182,   216,  -182,   217,  -182,
    -182,   168,  -182,  -182,  -182,  -182,  -182,    42,    43,   214,
    168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
    168,   168,   168,   168,  -182,   177,   381,   381,   177,    10,
    234,  -182,    44,   442,   381,  -182,   218,  -182,   381,  -182
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -182,  -182,  -182,  -182,  -182,    -1,    62,  -182,  -182,  -182,
    -182,   142,  -182,    -2,     4,   249,   119,   -97,  -182,  -153,
    -8,  -181,  -180,  -182,   -40,    -9,   180,   182,   189,   190,
    175,   170,  -182,  -182,   197,  -182,   165,  -182,   -25,  -182,
    -182,  -182,   131,  -179,  -178,  -177,  -175,  -174
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -94
static const yytype_int16 yytable[] =
{
    22,    18,    69,    70,   133,   194,    23,   229,   230,   232,
    233,   234,   199,   235,   236,    67,    68,   105,   144,    71,
    46,    72,    26,   218,    76,    99,    73,    64,   215,    34,
    35,   220,   100,     3,   217,    29,    30,   263,   221,   178,
    146,    92,    93,   219,    28,   149,    77,    25,    36,   117,
    106,    77,    74,    75,    37,    77,   115,    27,   102,    77,
    111,   108,   150,   118,   103,    77,   112,    38,    77,    39,
    77,    77,   114,   129,   256,   257,   265,    40,    41,    42,
    43,    31,   229,   230,   232,   233,   234,    44,   235,   236,
    186,   227,   170,    65,    95,    96,    97,    66,    77,   220,
    77,    77,    77,   260,   261,    32,   254,   187,   111,   108,
    4,   267,     5,     6,   112,   269,    33,    59,   170,    60,
    61,     8,     9,    10,    62,   170,    89,    90,    38,    13,
    14,    15,   102,   180,   142,   143,   181,   118,   103,   183,
    185,   107,    78,    79,     4,   190,     5,     6,   170,     4,
    80,     5,     6,    81,   116,     8,     9,    10,    73,   120,
    8,     9,    10,    13,    14,    15,   227,   130,    13,    14,
    15,   132,   101,   131,   134,   135,   136,   185,   137,   231,
    237,   139,   238,   140,   138,   145,   226,   225,   151,    34,
    35,   147,   148,   240,   241,   242,   243,   244,   245,   246,
    247,   248,   249,   250,   251,   252,   179,   188,    36,   189,
    192,   253,   191,   193,    37,   195,   170,   170,   196,   197,
    200,   201,   202,   170,   170,   216,    77,    38,   170,    39,
    82,    83,    84,    85,    86,    87,   239,    40,    41,    42,
    43,    59,   -92,   264,   255,   258,   259,    44,   222,   262,
    268,    24,   141,   177,   231,   266,   125,   121,   126,   128,
    122,   226,   225,     4,   152,     5,     6,   153,   123,   154,
    124,    34,    35,   155,     8,     9,    10,   182,   156,   157,
    0,   158,    13,    14,    15,     0,   159,     0,   127,     0,
    36,     0,     0,   132,   160,     0,    37,     0,     0,     0,
    0,     0,     0,     0,     0,     0,     0,     0,     0,    38,
    0,    39,     0,     0,     0,     0,     0,     0,     0,    40,
    41,    42,    43,     0,     4,   152,     5,     6,   153,    44,
    154,     0,    34,    35,   155,     8,     9,    10,     0,   156,
    157,     0,   158,    13,    14,    15,     0,   159,     0,     0,
    0,    36,     0,     0,   132,   198,     0,    37,     0,     0,
    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    38,     0,    39,     0,     0,     0,     0,     0,     0,     0,
    40,    41,    42,    43,     0,     4,   152,     5,     6,   153,
    44,   154,     0,    34,    35,   155,     8,     9,    10,     0,
    156,   157,     0,   158,    13,    14,    15,     0,   159,     0,
    0,     0,    36,     0,     0,   132,     0,     0,    37,     0,
    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    0,    38,     0,    39,     0,     0,     0,     0,     0,     0,
    0,    40,    41,    42,    43,     0,     4,   152,     5,     6,
    153,    44,   154,     0,    34,    35,   155,     8,     9,    10,
    0,   223,   224,     0,   158,    13,    14,    15,     0,     0,
    0,     0,     0,    36,     0,     0,   132,     0,     0,    37,
    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    0,     0,    38,     0,    39,     0,     0,     0,     0,     0,
    0,   -93,    40,    41,    42,    43,    72,     0,     0,     0,
    0,    73,    44,   203,   204,   205,   206,   207,   208,   209,
    210,   211,   212,   213,   214,     0,     0,     0,     4,     0,
    5,     6,     0,     0,     0,     7,     0,    74,    75,     8,
    9,    10,    11,     0,     0,    12,     0,    13,    14,    15
};

static const yytype_int16 yycheck[] =
{
    2,     2,    42,    43,   101,   158,     2,   188,   188,   188,
    188,   188,   165,   188,   188,    40,    41,    37,    30,    44,
    29,    31,     4,    30,    27,    30,    36,    36,    27,    12,
    13,    28,    37,     0,    27,    26,    27,    27,    35,   136,
    28,    70,    71,    30,     3,    34,    58,    51,    31,    32,
    70,    58,    62,    63,    37,    58,    65,    27,    60,    58,
    62,    62,    51,    72,    60,    58,    62,    50,    58,    52,
    58,    58,    32,    98,    32,    32,    32,    60,    61,    62,
    63,    50,   263,   263,   263,   263,   263,    70,   263,   263,
    34,   188,   132,    29,    72,    73,    74,    33,    58,    28,
    58,    58,    58,   256,   257,    50,    35,    51,   110,   110,
    4,   264,     6,     7,   110,   268,    50,    29,   158,    31,
    26,    15,    16,    17,    34,   165,    53,    54,    50,    23,
    24,    25,   134,   142,    26,    27,   145,   146,   134,   148,
    149,    35,    59,    56,     4,   154,     6,     7,   188,     4,
    55,     6,     7,    57,    50,    15,    16,    17,    36,    50,
    15,    16,    17,    23,    24,    25,   263,    26,    23,    24,
    25,    34,    32,    30,    28,    50,    32,   186,    27,   188,
    189,    27,   191,    35,    37,    29,   188,   188,    26,    12,
    13,    32,    29,   202,   203,   204,   205,   206,   207,   208,
    209,   210,   211,   212,   213,   214,    27,    31,    31,    31,
    27,   220,    31,    27,    37,    27,   256,   257,    27,    50,
    27,    27,    26,   263,   264,    27,    58,    50,   268,    52,
    64,    65,    66,    67,    68,    69,    14,    60,    61,    62,
    63,    29,    26,     9,    27,    31,   255,    70,   186,   258,
    32,     2,   110,   134,   263,   263,    81,    77,    88,    94,
    78,   263,   263,     4,     5,     6,     7,     8,    79,    10,
    80,    12,    13,    14,    15,    16,    17,   146,    19,    20,
    -1,    22,    23,    24,    25,    -1,    27,    -1,    91,    -1,
    31,    -1,    -1,    34,    35,    -1,    37,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
    61,    62,    63,    -1,     4,     5,     6,     7,     8,    70,
    10,    -1,    12,    13,    14,    15,    16,    17,    -1,    19,
    20,    -1,    22,    23,    24,    25,    -1,    27,    -1,    -1,
    -1,    31,    -1,    -1,    34,    35,    -1,    37,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    50,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    60,    61,    62,    63,    -1,     4,     5,     6,     7,     8,
    70,    10,    -1,    12,    13,    14,    15,    16,    17,    -1,
    19,    20,    -1,    22,    23,    24,    25,    -1,    27,    -1,
    -1,    -1,    31,    -1,    -1,    34,    -1,    -1,    37,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    50,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    60,    61,    62,    63,    -1,     4,     5,     6,     7,
    8,    70,    10,    -1,    12,    13,    14,    15,    16,    17,
    -1,    19,    20,    -1,    22,    23,    24,    25,    -1,    -1,
    -1,    -1,    -1,    31,    -1,    -1,    34,    -1,    -1,    37,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    50,    -1,    52,    -1,    -1,    -1,    -1,    -1,
    -1,    26,    60,    61,    62,    63,    31,    -1,    -1,    -1,
    -1,    36,    70,    38,    39,    40,    41,    42,    43,    44,
    45,    46,    47,    48,    49,    -1,    -1,    -1,     4,    -1,
    6,     7,    -1,    -1,    -1,    11,    -1,    62,    63,    15,
    16,    17,    18,    -1,    -1,    21,    -1,    23,    24,    25
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
    0,    76,    77,     0,     4,     6,     7,    11,    15,    16,
    17,    18,    21,    23,    24,    25,    78,    79,    80,    82,
    83,    84,    88,    89,    90,    51,     4,    27,     3,    26,
    27,    50,    50,    50,    12,    13,    31,    37,    50,    52,
    60,    61,    62,    63,    70,    99,   100,   101,   102,   103,
    104,   105,   106,   109,   111,   113,   114,   115,   116,    29,
    31,    26,    34,    85,   100,    29,    33,   113,   113,    99,
    99,   113,    31,    36,    62,    63,    27,    58,    59,    56,
    55,    57,    64,    65,    66,    67,    68,    69,   108,    53,
    54,   107,    70,    71,   110,    72,    73,    74,   112,    30,
    37,    32,    88,    89,    91,    37,    70,    35,    80,    86,
    87,    88,    89,    90,    32,   100,    50,    32,   100,   117,
    50,   101,   102,   103,   104,   105,   106,   109,   111,   113,
    26,    30,    34,    92,    28,    50,    32,    27,    37,    27,
    35,    86,    26,    27,    30,    29,    28,    32,    29,    34,
    51,    26,     5,     8,    10,    14,    19,    20,    22,    27,
    35,    80,    88,    89,    92,    93,    94,    96,    97,    98,
    99,   100,   118,   119,   120,   121,   122,    91,    92,    27,
    100,   100,   117,   100,    81,   100,    34,    51,    31,    31,
    100,    31,    27,    27,    94,    27,    27,    50,    35,    94,
    27,    27,    26,    38,    39,    40,    41,    42,    43,    44,
    45,    46,    47,    48,    49,    27,    27,    27,    30,    30,
    28,    35,    81,    19,    20,    80,    88,    92,    95,    96,
    97,   100,   118,   119,   120,   121,   122,   100,   100,    14,
    100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
    100,   100,   100,   100,    35,    27,    32,    32,    31,   100,
    94,    94,   100,    27,     9,    32,    95,    94,    32,    94
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
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
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
    while (YYID (0))
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
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print(FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print(yyoutput, yytype, yyvaluep, yylocationp)
FILE *yyoutput;
int yytype;
YYSTYPE const * const yyvaluep;
YYLTYPE const * const yylocationp;
#endif
{
    if(!yyvaluep)
        return;
        
    YYUSE(yylocationp);
# ifdef YYPRINT
    
    if(yytype < YYNTOKENS)
        YYPRINT(yyoutput, yytoknum[yytype], *yyvaluep);
        
# else
    YYUSE(yyoutput);
# endif
        
    switch(yytype)
    {
    default:
        break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print(FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print(yyoutput, yytype, yyvaluep, yylocationp)
FILE *yyoutput;
int yytype;
YYSTYPE const * const yyvaluep;
YYLTYPE const * const yylocationp;
#endif
{
    if(yytype < YYNTOKENS)
        YYFPRINTF(yyoutput, "token %s (", yytname[yytype]);
    else
        YYFPRINTF(yyoutput, "nterm %s (", yytname[yytype]);
        
    YY_LOCATION_PRINT(yyoutput, *yylocationp);
    YYFPRINTF(yyoutput, ": ");
    yy_symbol_value_print(yyoutput, yytype, yyvaluep, yylocationp);
    YYFPRINTF(yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print(yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print(yybottom, yytop)
yytype_int16 *yybottom;
yytype_int16 *yytop;
#endif
{
    YYFPRINTF(stderr, "Stack now");
    
    for(; yybottom <= yytop; yybottom++)
    {
        int yybot = *yybottom;
        YYFPRINTF(stderr, " %d", yybot);
    }
    
    YYFPRINTF(stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print(YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print(yyvsp, yylsp, yyrule)
YYSTYPE *yyvsp;
YYLTYPE *yylsp;
int yyrule;
#endif
{
    int yynrhs = yyr2[yyrule];
    int yyi;
    unsigned long int yylno = yyrline[yyrule];
    YYFPRINTF(stderr, "Reducing stack by rule %d (line %lu):\n",
              yyrule - 1, yylno);
              
    /* The symbols being reduced.  */
    for(yyi = 0; yyi < yynrhs; yyi++)
    {
        YYFPRINTF(stderr, "   $%d = ", yyi + 1);
        yy_symbol_print(stderr, yyrhs[yyprhs[yyrule] + yyi],
                        &(yyvsp[(yyi + 1) - (yynrhs)])
                        , &(yylsp[(yyi + 1) - (yynrhs)]));
        YYFPRINTF(stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

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
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen(const char *yystr)
#else
static YYSIZE_T
yystrlen(yystr)
const char *yystr;
#endif
{
    YYSIZE_T yylen;

    for(yylen = 0; yystr[yylen]; yylen++)
        continue;

    return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy(char *yydest, const char *yysrc)
#else
static char *
yystpcpy(yydest, yysrc)
char *yydest;
const char *yysrc;
#endif
{
    char *yyd = yydest;
    const char *yys = yysrc;

    while((*yyd++ = *yys++) != '\0')
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
yytnamerr(char *yyres, const char *yystr)
{
    if(*yystr == '"')
    {
        YYSIZE_T yyn = 0;
        char const *yyp = yystr;
        
        for(;;)
            switch(*++yyp)
            {
            case '\'':
            case ',':
                goto do_not_strip_quotes;
                
            case '\\':
                if(*++yyp != '\\')
                    goto do_not_strip_quotes;
                    
                /* Fall through.  */
            default:
                if(yyres)
                    yyres[yyn] = *yyp;
                    
                yyn++;
                break;
                
            case '"':
                if(yyres)
                    yyres[yyn] = '\0';
                    
                return yyn;
            }
            
do_not_strip_quotes:
        ;
    }
    
    if(! yyres)
        return yystrlen(yystr);
        
    return yystpcpy(yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error(char *yyresult, int yystate, int yychar)
{
    int yyn = yypact[yystate];
    
    if(!(YYPACT_NINF < yyn && yyn <= YYLAST))
        return 0;
    else
    {
        int yytype = YYTRANSLATE(yychar);
        YYSIZE_T yysize0 = yytnamerr(0, yytname[yytype]);
        YYSIZE_T yysize = yysize0;
        YYSIZE_T yysize1;
        int yysize_overflow = 0;
        enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
        char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
        int yyx;
        
# if 0
        /* This is so xgettext sees the translatable formats that are
        constructed on the fly.  */
        YY_("syntax error, unexpected %s");
        YY_("syntax error, unexpected %s, expecting %s");
        YY_("syntax error, unexpected %s, expecting %s or %s");
        YY_("syntax error, unexpected %s, expecting %s or %s or %s");
        YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
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
        int yychecklim = YYLAST - yyn + 1;
        int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        int yycount = 1;
        
        yyarg[0] = yytname[yytype];
        yyfmt = yystpcpy(yyformat, yyunexpected);
        
        for(yyx = yyxbegin; yyx < yyxend; ++yyx)
            if(yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
            {
                if(yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                {
                    yycount = 1;
                    yysize = yysize0;
                    yyformat[sizeof yyunexpected - 1] = '\0';
                    break;
                }
                
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr(0, yytname[yyx]);
                yysize_overflow |= (yysize1 < yysize);
                yysize = yysize1;
                yyfmt = yystpcpy(yyfmt, yyprefix);
                yyprefix = yyor;
            }
            
        yyf = YY_(yyformat);
        yysize1 = yysize + yystrlen(yyf);
        yysize_overflow |= (yysize1 < yysize);
        yysize = yysize1;
        
        if(yysize_overflow)
            return YYSIZE_MAXIMUM;
            
        if(yyresult)
        {
            /* Avoid sprintf, as that infringes on the user's name space.
               Don't have undefined behavior even if the translation
               produced a string with the wrong number of "%s"s.  */
            char *yyp = yyresult;
            int yyi = 0;
            
            while((*yyp = *yyf) != '\0')
            {
                if(*yyp == '%' && yyf[1] == 's' && yyi < yycount)
                {
                    yyp += yytnamerr(yyp, yyarg[yyi++]);
                    yyf += 2;
                }
                else
                {
                    yyp++;
                    yyf++;
                }
            }
        }
        
        return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct(const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct(yymsg, yytype, yyvaluep, yylocationp)
const char *yymsg;
int yytype;
YYSTYPE *yyvaluep;
YYLTYPE *yylocationp;
#endif
{
    YYUSE(yyvaluep);
    YYUSE(yylocationp);
    
    if(!yymsg)
        yymsg = "Deleting";
        
    YY_SYMBOL_PRINT(yymsg, yytype, yyvaluep, yylocationp);
    
    switch(yytype)
    {
    
    default:
        break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse(void *YYPARSE_PARAM);
#else
int yyparse();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse(void);
#else
int yyparse();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse(void *YYPARSE_PARAM)
#else
int
yyparse(YYPARSE_PARAM)
void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse(void)
#else
int
yyparse()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;
    
    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.
    
       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */
    
    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;
    
    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;
    
    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;
    
    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[2];
    
    YYSIZE_T yystacksize;
    
    int yyn;
    int yyresult;
    /* Lookahead token as an internal (translated) token number.  */
    int yytoken;
    /* The variables used to return semantic value and location from the
       action routines.  */
    YYSTYPE yyval;
    YYLTYPE yyloc;
    
#if YYERROR_VERBOSE
    /* Buffer for error messages, and its allocated size.  */
    char yymsgbuf[128];
    char *yymsg = yymsgbuf;
    YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif
    
#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))
    
    /* The number of symbols on the RHS of the reduced rule.
       Keep to zero when no symbol should be popped.  */
    int yylen = 0;
    
    yytoken = 0;
    yyss = yyssa;
    yyvs = yyvsa;
    yyls = yylsa;
    yystacksize = YYINITDEPTH;
    
    YYDPRINTF((stderr, "Starting parse\n"));
    
    yystate = 0;
    yyerrstatus = 0;
    yynerrs = 0;
    yychar = YYEMPTY; /* Cause a token to be read.  */
    
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
    yylloc.first_column = yylloc.last_column = 1;
#endif
    
    goto yysetstate;
    
    /*------------------------------------------------------------.
    | yynewstate -- Push a new state, which is found in yystate.  |
    `------------------------------------------------------------*/
yynewstate:
    /* In all cases, when you get here, the value and location stacks
       have just been pushed.  So pushing a state here evens the stacks.  */
    yyssp++;
    
yysetstate:
    *yyssp = yystate;
    
    if(yyss + yystacksize - 1 <= yyssp)
    {
        /* Get the current used size of the three stacks, in elements.  */
        YYSIZE_T yysize = yyssp - yyss + 1;
        
#ifdef yyoverflow
        {
            /* Give user a chance to reallocate the stack.  Use copies of
               these so that the &'s don't force the real ones into
               memory.  */
            YYSTYPE *yyvs1 = yyvs;
            yytype_int16 *yyss1 = yyss;
            YYLTYPE *yyls1 = yyls;
            
            /* Each stack pointer address is followed by the size of the
               data in use in that stack, in bytes.  This used to be a
               conditional around just the two extra args, but that might
               be undefined if yyoverflow is a macro.  */
            yyoverflow(YY_("memory exhausted"),
                       &yyss1, yysize * sizeof(*yyssp),
                       &yyvs1, yysize * sizeof(*yyvsp),
                       &yyls1, yysize * sizeof(*yylsp),
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
        if(YYMAXDEPTH <= yystacksize)
            goto yyexhaustedlab;
        
        yystacksize *= 2;
        
        if(YYMAXDEPTH < yystacksize)
            yystacksize = YYMAXDEPTH;
        
        {
            yytype_int16 *yyss1 = yyss;
            union yyalloc *yyptr =
                        (union yyalloc *) YYSTACK_ALLOC(YYSTACK_BYTES(yystacksize));
        
            if(! yyptr)
                goto yyexhaustedlab;
        
            YYSTACK_RELOCATE(yyss_alloc, yyss);
            YYSTACK_RELOCATE(yyvs_alloc, yyvs);
            YYSTACK_RELOCATE(yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        
            if(yyss1 != yyssa)
                YYSTACK_FREE(yyss1);
        }
# endif
#endif /* no yyoverflow */
        
        yyssp = yyss + yysize - 1;
        yyvsp = yyvs + yysize - 1;
        yylsp = yyls + yysize - 1;
        
        YYDPRINTF((stderr, "Stack size increased to %lu\n",
                   (unsigned long int) yystacksize));
                   
        if(yyss + yystacksize - 1 <= yyssp)
            YYABORT;
    }
    
    YYDPRINTF((stderr, "Entering state %d\n", yystate));
    
    if(yystate == YYFINAL)
        YYACCEPT;
        
    goto yybackup;
    
    /*-----------.
    | yybackup.  |
    `-----------*/
yybackup:

    /* Do appropriate processing given the current state.  Read a
       lookahead token if we need one and don't already have one.  */
    
    /* First try to decide what to do without reference to lookahead token.  */
    yyn = yypact[yystate];
    
    if(yyn == YYPACT_NINF)
        goto yydefault;
        
    /* Not known => get a lookahead token if don't already have one.  */
    
    /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
    if(yychar == YYEMPTY)
    {
        YYDPRINTF((stderr, "Reading a token: "));
        yychar = YYLEX;
    }
    
    if(yychar <= YYEOF)
    {
        yychar = yytoken = YYEOF;
        YYDPRINTF((stderr, "Now at end of input.\n"));
    }
    else
    {
        yytoken = YYTRANSLATE(yychar);
        YY_SYMBOL_PRINT("Next token is", yytoken, &yylval, &yylloc);
    }
    
    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    
    if(yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
        goto yydefault;
        
    yyn = yytable[yyn];
    
    if(yyn <= 0)
    {
        if(yyn == 0 || yyn == YYTABLE_NINF)
            goto yyerrlab;
            
        yyn = -yyn;
        goto yyreduce;
    }
    
    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if(yyerrstatus)
        yyerrstatus--;
        
    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT("Shifting", yytoken, &yylval, &yylloc);
    
    /* Discard the shifted token.  */
    yychar = YYEMPTY;
    
    yystate = yyn;
    *++yyvsp = yylval;
    *++yylsp = yylloc;
    goto yynewstate;
    
    
    /*-----------------------------------------------------------.
    | yydefault -- do the default action for the current state.  |
    `-----------------------------------------------------------*/
yydefault:
    yyn = yydefact[yystate];
    
    if(yyn == 0)
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
    
    /* Default location.  */
    YYLLOC_DEFAULT(yyloc, (yylsp - yylen), yylen);
    YY_REDUCE_PRINT(yyn);
    
    switch(yyn)
    {
    case 2:
    
        /* Line 1455 of yacc.c  */
#line 107 "ffscript.ypp"
    {resAST = new ASTProgram((ASTDeclList *)(yyvsp[(1) - (1)]), (yylsp[(1) - (1)]));;}
    break;
    
    case 3:
    
        /* Line 1455 of yacc.c  */
#line 110 "ffscript.ypp"
    {
        ASTDeclList *list = (ASTDeclList *)(yyvsp[(1) - (2)]);
        list->addDeclaration((ASTDecl *)(yyvsp[(2) - (2)]));
        (yyval) = list;;
    }
    break;
    
    case 4:
    
        /* Line 1455 of yacc.c  */
#line 113 "ffscript.ypp"
    {(yyval) = new ASTDeclList(noloc);;}
    break;
    
    case 5:
    
        /* Line 1455 of yacc.c  */
#line 116 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 6:
    
        /* Line 1455 of yacc.c  */
#line 117 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 7:
    
        /* Line 1455 of yacc.c  */
#line 118 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 8:
    
        /* Line 1455 of yacc.c  */
#line 119 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 9:
    
        /* Line 1455 of yacc.c  */
#line 120 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (2)]);;}
    break;
    
    case 10:
    
        /* Line 1455 of yacc.c  */
#line 121 "ffscript.ypp"
    {
        ASTVarDecl *vd = (ASTVarDecl *)(yyvsp[(1) - (4)]);
        Clone c;
        vd->getType()->execute(c,NULL);
        (yyval) = new ASTVarDeclInitializer((ASTType *)c.getResult(), vd->getName(), (ASTExpr *)(yyvsp[(3) - (4)]),(yylsp[(1) - (4)]));
        delete vd;;
    }
    break;
    
    case 11:
    
        /* Line 1455 of yacc.c  */
#line 126 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (2)]);;}
    break;
    
    case 12:
    
        /* Line 1455 of yacc.c  */
#line 129 "ffscript.ypp"
    {
        ASTString *name = (ASTString *)(yyvsp[(3) - (6)]);
        ASTFloat *val = (ASTFloat *)(yyvsp[(5) - (6)]);
        (yyval) = new ASTConstDecl(name->getValue(), val,(yylsp[(1) - (6)]));
        delete name;;
    }
    break;
    
    case 13:
    
        /* Line 1455 of yacc.c  */
#line 133 "ffscript.ypp"
    {
        ASTString *name = (ASTString *)(yyvsp[(3) - (7)]);
        ASTFloat *val = (ASTFloat *)(yyvsp[(6) - (7)]);
        val->set_negative(true);
        (yyval) = new ASTConstDecl(name->getValue(), val,(yylsp[(1) - (7)]));
        delete name;;
    }
    break;
    
    case 14:
    
        /* Line 1455 of yacc.c  */
#line 139 "ffscript.ypp"
    {
        ASTType *type = (ASTType *)(yyvsp[(1) - (5)]);
        ASTString *name = (ASTString *)(yyvsp[(2) - (5)]);
        ASTExpr *size = (ASTExpr *)(yyvsp[(4) - (5)]);
        (yyval) = new ASTArrayDecl(type, name->getValue(), size, false, NULL, (yylsp[(1) - (5)]));
        delete name;;
    }
    break;
    
    case 15:
    
        /* Line 1455 of yacc.c  */
#line 145 "ffscript.ypp"
    {
        ASTType *type = (ASTType *)(yyvsp[(1) - (9)]);
        ASTString *name = (ASTString *)(yyvsp[(2) - (9)]);
        ASTFloat *size = (ASTFloat *)(yyvsp[(4) - (9)]);
        ASTArrayList *list = (ASTArrayList *)(yyvsp[(8) - (9)]);
        (yyval) = new ASTArrayDecl(type, name->getValue(), size, false, list, (yylsp[(1) - (9)]));
        delete name;;
    }
    break;
    
    case 16:
    
        /* Line 1455 of yacc.c  */
#line 151 "ffscript.ypp"
    {
        ASTType *type = (ASTType *)(yyvsp[(1) - (8)]);
        ASTString *name = (ASTString *)(yyvsp[(2) - (8)]);
        ASTArrayList *list = (ASTArrayList *)(yyvsp[(7) - (8)]);
        char val[15];
        sprintf(val, "%d", list->getList().size());
        ASTFloat *size = new ASTFloat(val, 0, (yylsp[(1) - (8)]));
        (yyval) = new ASTArrayDecl(type, name->getValue(), size, false, list, (yylsp[(1) - (8)]));
        delete name;;
    }
    break;
    
    case 17:
    
        /* Line 1455 of yacc.c  */
#line 160 "ffscript.ypp"
    {
        ASTType *type = (ASTType *)(yyvsp[(1) - (7)]);
        ASTString *name = (ASTString *)(yyvsp[(2) - (7)]);
        ASTFloat *size = (ASTFloat *)(yyvsp[(4) - (7)]);
        ASTArrayList *list = new ASTArrayList((yylsp[(1) - (7)]));
        list->makeString();
        ASTString *string = (ASTString *)(yyvsp[(7) - (7)]);
        
        for(unsigned int i=1; i < string->getValue().length()-1; i++)
            list->addParam(new ASTNumConstant(new ASTFloat(long((string->getValue())[i]), 0, (yylsp[(1) - (7)])), (yylsp[(1) - (7)])));
            
        list->addParam(new ASTNumConstant(new ASTFloat(0L, 0, (yylsp[(1) - (7)])), (yylsp[(1) - (7)])));
        (yyval) = new ASTArrayDecl(type, name->getValue(), size, false, list, (yylsp[(1) - (7)]));
        delete name;;
    }
    break;
    
    case 18:
    
        /* Line 1455 of yacc.c  */
#line 174 "ffscript.ypp"
    {
        ASTType *type = (ASTType *)(yyvsp[(1) - (6)]);
        ASTString *name = (ASTString *)(yyvsp[(2) - (6)]);
        ASTArrayList *list = new ASTArrayList((yylsp[(1) - (6)]));
        list->makeString();
        ASTString *string = (ASTString *)(yyvsp[(6) - (6)]);
        ASTFloat *size = new ASTFloat(string->getValue().length()-1, 0, (yylsp[(1) - (6)]));
        
        for(unsigned int i=1; i < string->getValue().length()-1; i++)
            list->addParam(new ASTNumConstant(new ASTFloat(long((string->getValue())[i]), 0, (yylsp[(1) - (6)])), (yylsp[(1) - (6)])));
            
        list->addParam(new ASTNumConstant(new ASTFloat(0L, 0, (yylsp[(1) - (6)])), (yylsp[(1) - (6)])));
        (yyval) = new ASTArrayDecl(type, name->getValue(), size, false, list, (yylsp[(1) - (6)]));
        delete name;;
    }
    break;
    
    case 19:
    
        /* Line 1455 of yacc.c  */
#line 191 "ffscript.ypp"
    {
        ASTArrayList *al = (ASTArrayList *)(yyvsp[(1) - (3)]);
        al->addParam((ASTExpr*)(yyvsp[(3) - (3)]));
        (yyval)=al;;
    }
    break;
    
    case 20:
    
        /* Line 1455 of yacc.c  */
#line 194 "ffscript.ypp"
    {
        ASTArrayList *al = new ASTArrayList((yylsp[(1) - (1)]));
        al->addParam((ASTExpr *)(yyvsp[(1) - (1)]));
        (yyval)=al;;
    }
    break;
    
    case 21:
    
        /* Line 1455 of yacc.c  */
#line 199 "ffscript.ypp"
    {
        ASTType *type = (ASTType *)(yyvsp[(1) - (4)]);
        int scripttype; //Itemdata pointer instead of item pointer
        ExtractType temp;
        type->execute(temp, &scripttype);
        
        if(scripttype == ScriptParser::TYPE_ITEM)
        {
            ASTType *t = type; //so we can keep the locationdata before deleting
            type = new ASTTypeItemclass(type->getLocation());
            delete t;
        }
        
        ASTString *name = (ASTString *)(yyvsp[(3) - (4)]);
        ASTDeclList *sblock = (ASTDeclList *)(yyvsp[(4) - (4)]);
        (yyval) = new ASTScript(type, name->getValue(), sblock,(yylsp[(1) - (4)]));
        delete name;;
    }
    break;
    
    case 22:
    
        /* Line 1455 of yacc.c  */
#line 215 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 23:
    
        /* Line 1455 of yacc.c  */
#line 216 "ffscript.ypp"
    {(yyval) = new ASTTypeGlobal((yylsp[(1) - (1)]));;}
    break;
    
    case 24:
    
        /* Line 1455 of yacc.c  */
#line 219 "ffscript.ypp"
    {
        ASTString *str = (ASTString *)(yyvsp[(2) - (2)]);
        (yyval) = new ASTImportDecl(str->getValue(),(yylsp[(1) - (2)]));
        delete str;;
    }
    break;
    
    case 25:
    
        /* Line 1455 of yacc.c  */
#line 224 "ffscript.ypp"
    {(yyval) = (yyvsp[(2) - (3)]);;}
    break;
    
    case 26:
    
        /* Line 1455 of yacc.c  */
#line 225 "ffscript.ypp"
    {(yyval) = new ASTDeclList((yylsp[(1) - (2)]));;}
    break;
    
    case 27:
    
        /* Line 1455 of yacc.c  */
#line 228 "ffscript.ypp"
    {
        ASTDeclList *dl = (ASTDeclList *)(yyvsp[(2) - (2)]);
        dl->addDeclaration((ASTDecl *)(yyvsp[(1) - (2)]));
        (yyval) = dl;;
    }
    break;
    
    case 28:
    
        /* Line 1455 of yacc.c  */
#line 231 "ffscript.ypp"
    {
        ASTDeclList *dl = new ASTDeclList((yylsp[(1) - (1)]));
        dl->addDeclaration((ASTDecl *)(yyvsp[(1) - (1)]));
        (yyval) = dl;;
    }
    break;
    
    case 29:
    
        /* Line 1455 of yacc.c  */
#line 236 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (2)]);;}
    break;
    
    case 30:
    
        /* Line 1455 of yacc.c  */
#line 237 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (2)]);;}
    break;
    
    case 31:
    
        /* Line 1455 of yacc.c  */
#line 238 "ffscript.ypp"
    {
        ASTVarDecl *vd = (ASTVarDecl *)(yyvsp[(1) - (4)]);
        Clone c;
        vd->getType()->execute(c,NULL);
        (yyval) = new ASTVarDeclInitializer((ASTType *)c.getResult(), vd->getName(), (ASTExpr *)(yyvsp[(3) - (4)]),(yylsp[(1) - (4)]));
        delete vd;;
    }
    break;
    
    case 32:
    
        /* Line 1455 of yacc.c  */
#line 243 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 33:
    
        /* Line 1455 of yacc.c  */
#line 246 "ffscript.ypp"
    {
        ASTType *type = (ASTType *)(yyvsp[(1) - (2)]);
        ASTString *name = (ASTString *)(yyvsp[(2) - (2)]);
        (yyval) = new ASTVarDecl(type, name->getValue(),(yylsp[(1) - (2)]));
        delete name;;
    }
    break;
    
    case 34:
    
        /* Line 1455 of yacc.c  */
#line 252 "ffscript.ypp"
    {(yyval) = new ASTTypeFloat((yylsp[(1) - (1)]));;}
    break;
    
    case 35:
    
        /* Line 1455 of yacc.c  */
#line 253 "ffscript.ypp"
    {(yyval) = new ASTTypeBool((yylsp[(1) - (1)]));;}
    break;
    
    case 36:
    
        /* Line 1455 of yacc.c  */
#line 254 "ffscript.ypp"
    {(yyval) = new ASTTypeVoid((yylsp[(1) - (1)]));;}
    break;
    
    case 37:
    
        /* Line 1455 of yacc.c  */
#line 255 "ffscript.ypp"
    {(yyval) = new ASTTypeFFC((yylsp[(1) - (1)]));;}
    break;
    
    case 38:
    
        /* Line 1455 of yacc.c  */
#line 256 "ffscript.ypp"
    {(yyval) = new ASTTypeItem((yylsp[(1) - (1)]));;}
    break;
    
    case 39:
    
        /* Line 1455 of yacc.c  */
#line 257 "ffscript.ypp"
    {(yyval) = new ASTTypeItemclass((yylsp[(1) - (1)]));;}
    break;
    
    case 40:
    
        /* Line 1455 of yacc.c  */
#line 258 "ffscript.ypp"
    {(yyval) = new ASTTypeNPC((yylsp[(1) - (1)]));;}
    break;
    
    case 41:
    
        /* Line 1455 of yacc.c  */
#line 259 "ffscript.ypp"
    {(yyval) = new ASTTypeLWpn((yylsp[(1) - (1)]));;}
    break;
    
    case 42:
    
        /* Line 1455 of yacc.c  */
#line 260 "ffscript.ypp"
    {(yyval) = new ASTTypeEWpn((yylsp[(1) - (1)]));;}
    break;
    
    case 43:
    
        /* Line 1455 of yacc.c  */
#line 263 "ffscript.ypp"
    {
        ASTFuncDecl *fd = (ASTFuncDecl *)(yyvsp[(4) - (6)]);
        ASTType *rettype = (ASTType *)(yyvsp[(1) - (6)]);
        ASTString *name = (ASTString *)(yyvsp[(2) - (6)]);
        ASTBlock *block = (ASTBlock *)(yyvsp[(6) - (6)]);
        fd->setName(name->getValue());
        delete name;
        fd->setReturnType(rettype);
        fd->setBlock(block);
        (yyval)=fd;;
    }
    break;
    
    case 44:
    
        /* Line 1455 of yacc.c  */
#line 272 "ffscript.ypp"
    {
        ASTFuncDecl *fd = new ASTFuncDecl((yylsp[(1) - (5)]));
        ASTType *rettype = (ASTType *)(yyvsp[(1) - (5)]);
        ASTString *name = (ASTString *)(yyvsp[(2) - (5)]);
        ASTBlock *block = (ASTBlock *)(yyvsp[(5) - (5)]);
        fd->setName(name->getValue());
        delete name;
        fd->setReturnType(rettype);
        fd->setBlock(block);
        (yyval)=fd;;
    }
    break;
    
    case 45:
    
        /* Line 1455 of yacc.c  */
#line 283 "ffscript.ypp"
    {
        ASTFuncDecl *fd = (ASTFuncDecl *)(yyvsp[(3) - (3)]);
        fd->addParam((ASTVarDecl *)(yyvsp[(1) - (3)]));
        (yyval)=fd;;
    }
    break;
    
    case 46:
    
        /* Line 1455 of yacc.c  */
#line 286 "ffscript.ypp"
    {
        ASTFuncDecl *fd = new ASTFuncDecl((yylsp[(1) - (1)]));
        fd->addParam((ASTVarDecl *)(yyvsp[(1) - (1)]));
        (yyval)=fd;;
    }
    break;
    
    case 47:
    
        /* Line 1455 of yacc.c  */
#line 291 "ffscript.ypp"
    {(yyval)=(yyvsp[(2) - (3)]);;}
    break;
    
    case 48:
    
        /* Line 1455 of yacc.c  */
#line 292 "ffscript.ypp"
    {(yyval) = new ASTBlock((yylsp[(1) - (2)]));;}
    break;
    
    case 49:
    
        /* Line 1455 of yacc.c  */
#line 295 "ffscript.ypp"
    {
        ASTBlock *block = (ASTBlock *)(yyvsp[(1) - (2)]);
        ASTStmt *stmt = (ASTStmt *)(yyvsp[(2) - (2)]);
        block->addStatement(stmt);
        (yyval) = block;;
    }
    break;
    
    case 50:
    
        /* Line 1455 of yacc.c  */
#line 299 "ffscript.ypp"
    {
        ASTBlock *block = new ASTBlock((yylsp[(1) - (1)]));
        ASTStmt *stmt = (ASTStmt *)(yyvsp[(1) - (1)]);
        block->addStatement(stmt);
        (yyval) = block;;
    }
    break;
    
    case 51:
    
        /* Line 1455 of yacc.c  */
#line 305 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (2)]);;}
    break;
    
    case 52:
    
        /* Line 1455 of yacc.c  */
#line 306 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (2)]);;}
    break;
    
    case 53:
    
        /* Line 1455 of yacc.c  */
#line 307 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (2)]);;}
    break;
    
    case 54:
    
        /* Line 1455 of yacc.c  */
#line 308 "ffscript.ypp"
    {(yyval)=(yyvsp[(1) - (2)]);;}
    break;
    
    case 55:
    
        /* Line 1455 of yacc.c  */
#line 309 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 56:
    
        /* Line 1455 of yacc.c  */
#line 310 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 57:
    
        /* Line 1455 of yacc.c  */
#line 311 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 58:
    
        /* Line 1455 of yacc.c  */
#line 312 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (2)]);;}
    break;
    
    case 59:
    
        /* Line 1455 of yacc.c  */
#line 313 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 60:
    
        /* Line 1455 of yacc.c  */
#line 314 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 61:
    
        /* Line 1455 of yacc.c  */
#line 315 "ffscript.ypp"
    {(yyval) = new ASTStmtEmpty((yylsp[(1) - (1)]));;}
    break;
    
    case 62:
    
        /* Line 1455 of yacc.c  */
#line 316 "ffscript.ypp"
    {(yyval)=(yyvsp[(1) - (2)]);;}
    break;
    
    case 63:
    
        /* Line 1455 of yacc.c  */
#line 317 "ffscript.ypp"
    {(yyval) = new ASTStmtBreak((yylsp[(1) - (2)]));;}
    break;
    
    case 64:
    
        /* Line 1455 of yacc.c  */
#line 318 "ffscript.ypp"
    {(yyval) = new ASTStmtContinue((yylsp[(1) - (2)]));;}
    break;
    
    case 65:
    
        /* Line 1455 of yacc.c  */
#line 321 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 66:
    
        /* Line 1455 of yacc.c  */
#line 322 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 67:
    
        /* Line 1455 of yacc.c  */
#line 323 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 68:
    
        /* Line 1455 of yacc.c  */
#line 324 "ffscript.ypp"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;
    
    case 69:
    
        /* Line 1455 of yacc.c  */
#line 325 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 70:
    
        /* Line 1455 of yacc.c  */
#line 326 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 71:
    
        /* Line 1455 of yacc.c  */
#line 327 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 72:
    
        /* Line 1455 of yacc.c  */
#line 328 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 73:
    
        /* Line 1455 of yacc.c  */
#line 329 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 74:
    
        /* Line 1455 of yacc.c  */
#line 330 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 75:
    
        /* Line 1455 of yacc.c  */
#line 331 "ffscript.ypp"
    {(yyval) = new ASTStmtEmpty(noloc);;}
    break;
    
    case 76:
    
        /* Line 1455 of yacc.c  */
#line 332 "ffscript.ypp"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;
    
    case 77:
    
        /* Line 1455 of yacc.c  */
#line 333 "ffscript.ypp"
    {(yyval) = new ASTStmtBreak((yylsp[(1) - (1)]));;}
    break;
    
    case 78:
    
        /* Line 1455 of yacc.c  */
#line 334 "ffscript.ypp"
    {(yyval) = new ASTStmtContinue((yylsp[(1) - (1)]));;}
    break;
    
    case 79:
    
        /* Line 1455 of yacc.c  */
#line 337 "ffscript.ypp"
    {SHORTCUT(ASTExprPlus,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(yyval),(yylsp[(1) - (3)]),(yylsp[(2) - (3)])) ;}
    break;
    
    case 80:
    
        /* Line 1455 of yacc.c  */
#line 338 "ffscript.ypp"
    {SHORTCUT(ASTExprMinus,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(yyval),(yylsp[(1) - (3)]),(yylsp[(2) - (3)])) ;}
    break;
    
    case 81:
    
        /* Line 1455 of yacc.c  */
#line 339 "ffscript.ypp"
    {SHORTCUT(ASTExprTimes,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(yyval),(yylsp[(1) - (3)]),(yylsp[(2) - (3)])) ;}
    break;
    
    case 82:
    
        /* Line 1455 of yacc.c  */
#line 340 "ffscript.ypp"
    {SHORTCUT(ASTExprDivide,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(yyval),(yylsp[(1) - (3)]),(yylsp[(2) - (3)])) ;}
    break;
    
    case 83:
    
        /* Line 1455 of yacc.c  */
#line 341 "ffscript.ypp"
    {SHORTCUT(ASTExprAnd,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(yyval),(yylsp[(1) - (3)]),(yylsp[(2) - (3)])) ;}
    break;
    
    case 84:
    
        /* Line 1455 of yacc.c  */
#line 342 "ffscript.ypp"
    {SHORTCUT(ASTExprOr,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(yyval),(yylsp[(1) - (3)]),(yylsp[(2) - (3)])) ;}
    break;
    
    case 85:
    
        /* Line 1455 of yacc.c  */
#line 343 "ffscript.ypp"
    {SHORTCUT(ASTExprBitAnd,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(yyval),(yylsp[(1) - (3)]),(yylsp[(2) - (3)])) ;}
    break;
    
    case 86:
    
        /* Line 1455 of yacc.c  */
#line 344 "ffscript.ypp"
    {SHORTCUT(ASTExprBitOr,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(yyval),(yylsp[(1) - (3)]),(yylsp[(2) - (3)])) ;}
    break;
    
    case 87:
    
        /* Line 1455 of yacc.c  */
#line 345 "ffscript.ypp"
    {SHORTCUT(ASTExprBitXor,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(yyval),(yylsp[(1) - (3)]),(yylsp[(2) - (3)])) ;}
    break;
    
    case 88:
    
        /* Line 1455 of yacc.c  */
#line 346 "ffscript.ypp"
    {SHORTCUT(ASTExprLShift,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(yyval),(yylsp[(1) - (3)]),(yylsp[(2) - (3)])) ;}
    break;
    
    case 89:
    
        /* Line 1455 of yacc.c  */
#line 347 "ffscript.ypp"
    {SHORTCUT(ASTExprRShift,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(yyval),(yylsp[(1) - (3)]),(yylsp[(2) - (3)])) ;}
    break;
    
    case 90:
    
        /* Line 1455 of yacc.c  */
#line 348 "ffscript.ypp"
    {SHORTCUT(ASTExprModulo,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(yyval),(yylsp[(1) - (3)]),(yylsp[(2) - (3)])) ;}
    break;
    
    case 91:
    
        /* Line 1455 of yacc.c  */
#line 352 "ffscript.ypp"
    {(yyval) = new ASTStmtAssign((ASTStmt *)(yyvsp[(1) - (3)]), (ASTExpr *)(yyvsp[(3) - (3)]),(yylsp[(1) - (3)]));;}
    break;
    
    case 92:
    
        /* Line 1455 of yacc.c  */
#line 355 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 93:
    
        /* Line 1455 of yacc.c  */
#line 356 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 94:
    
        /* Line 1455 of yacc.c  */
#line 359 "ffscript.ypp"
    {
        ASTString *lval = (ASTString *)(yyvsp[(1) - (3)]);
        ASTString *rval = (ASTString *)(yyvsp[(3) - (3)]);
        (yyval) = new ASTExprDot(lval->getValue(), rval->getValue(),(yylsp[(1) - (3)]));
        delete lval;
        delete rval;;
    }
    break;
    
    case 95:
    
        /* Line 1455 of yacc.c  */
#line 363 "ffscript.ypp"
    {
        ASTString *name = (ASTString *)(yyvsp[(1) - (4)]);
        ASTExpr *num = (ASTExpr *)(yyvsp[(3) - (4)]);
        ASTExprArray *ar = new ASTExprArray("", name->getValue(), (yylsp[(1) - (4)]));
        ar->setIndex(num);
        (yyval) = ar;
        delete name;;
    }
    break;
    
    case 96:
    
        /* Line 1455 of yacc.c  */
#line 369 "ffscript.ypp"
    {
        ASTString *name = (ASTString *)(yyvsp[(1) - (6)]);
        ASTString *name2 = (ASTString *)(yyvsp[(3) - (6)]);
        ASTExpr *num = (ASTExpr *)(yyvsp[(5) - (6)]);
        ASTExprArray *ar = new ASTExprArray(name->getValue(), name2->getValue(), (yylsp[(1) - (6)]));
        ar->setIndex(num);
        (yyval) = ar;
        delete name;
        delete name2;;
    }
    break;
    
    case 97:
    
        /* Line 1455 of yacc.c  */
#line 377 "ffscript.ypp"
    {
        ASTString *rval = (ASTString *)(yyvsp[(1) - (1)]);
        (yyval) = new ASTExprDot("", rval->getValue(),(yylsp[(1) - (1)]));
        delete rval;;
    }
    break;
    
    case 98:
    
        /* Line 1455 of yacc.c  */
#line 380 "ffscript.ypp"
    {
        ASTExpr *id = (ASTExpr *)(yyvsp[(1) - (3)]);
        ASTString *rval = (ASTString *)(yyvsp[(3) - (3)]);
        (yyval) = new ASTExprArrow(id, rval->getValue(), (yylsp[(1) - (3)]));
        delete rval;;
    }
    break;
    
    case 99:
    
        /* Line 1455 of yacc.c  */
#line 384 "ffscript.ypp"
    {
        ASTExpr *id = (ASTExpr *)(yyvsp[(1) - (6)]);
        ASTString *rval = (ASTString *)(yyvsp[(3) - (6)]);
        ASTExpr *num = (ASTExpr *)(yyvsp[(5) - (6)]);
        ASTExprArrow *res = new ASTExprArrow(id, rval->getValue(), (yylsp[(1) - (6)]));
        res->setIndex(num);
        (yyval) = res;;
    }
    break;
    
    case 100:
    
        /* Line 1455 of yacc.c  */
#line 392 "ffscript.ypp"
    {
        ASTLogExpr *e = new ASTExprOr((yylsp[(2) - (3)]));
        ASTExpr *left = (ASTExpr *)(yyvsp[(1) - (3)]);
        ASTExpr *right = (ASTExpr *)(yyvsp[(3) - (3)]);
        e->setFirstOperand(left);
        e->setSecondOperand(right);
        (yyval)=e;;
    }
    break;
    
    case 101:
    
        /* Line 1455 of yacc.c  */
#line 398 "ffscript.ypp"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;
    
    case 102:
    
        /* Line 1455 of yacc.c  */
#line 401 "ffscript.ypp"
    {
        ASTLogExpr *e = new ASTExprAnd((yylsp[(2) - (3)]));
        ASTExpr *left = (ASTExpr *)(yyvsp[(1) - (3)]);
        ASTExpr *right = (ASTExpr *)(yyvsp[(3) - (3)]);
        e->setFirstOperand(left);
        e->setSecondOperand(right);
        (yyval)=e;;
    }
    break;
    
    case 103:
    
        /* Line 1455 of yacc.c  */
#line 407 "ffscript.ypp"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;
    
    case 104:
    
        /* Line 1455 of yacc.c  */
#line 410 "ffscript.ypp"
    {
        ASTBitExpr *e = new ASTExprBitOr((yylsp[(2) - (3)]));
        ASTExpr *left = (ASTExpr *)(yyvsp[(1) - (3)]);
        ASTExpr *right = (ASTExpr *)(yyvsp[(3) - (3)]);
        e->setFirstOperand(left);
        e->setSecondOperand(right);
        (yyval)=e;;
    }
    break;
    
    case 105:
    
        /* Line 1455 of yacc.c  */
#line 416 "ffscript.ypp"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;
    
    case 106:
    
        /* Line 1455 of yacc.c  */
#line 419 "ffscript.ypp"
    {
        ASTBitExpr *e = new ASTExprBitXor((yylsp[(2) - (3)]));
        ASTExpr *left = (ASTExpr *)(yyvsp[(1) - (3)]);
        ASTExpr *right = (ASTExpr *)(yyvsp[(3) - (3)]);
        e->setFirstOperand(left);
        e->setSecondOperand(right);
        (yyval)=e;;
    }
    break;
    
    case 107:
    
        /* Line 1455 of yacc.c  */
#line 425 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 108:
    
        /* Line 1455 of yacc.c  */
#line 428 "ffscript.ypp"
    {
        ASTBitExpr *e = new ASTExprBitAnd((yylsp[(2) - (3)]));
        ASTExpr *left = (ASTExpr *)(yyvsp[(1) - (3)]);
        ASTExpr *right = (ASTExpr *)(yyvsp[(3) - (3)]);
        e->setFirstOperand(left);
        e->setSecondOperand(right);
        (yyval)=e;;
    }
    break;
    
    case 109:
    
        /* Line 1455 of yacc.c  */
#line 434 "ffscript.ypp"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;
    
    case 110:
    
        /* Line 1455 of yacc.c  */
#line 437 "ffscript.ypp"
    {
        ASTRelExpr *e = (ASTRelExpr *)(yyvsp[(2) - (3)]);
        ASTExpr *left = (ASTExpr *)(yyvsp[(1) - (3)]);
        ASTExpr *right = (ASTExpr *)(yyvsp[(3) - (3)]);
        e->setFirstOperand(left);
        e->setSecondOperand(right);
        (yyval)=e;;
    }
    break;
    
    case 111:
    
        /* Line 1455 of yacc.c  */
#line 443 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 112:
    
        /* Line 1455 of yacc.c  */
#line 446 "ffscript.ypp"
    {
        ASTShiftExpr *e = (ASTShiftExpr *)(yyvsp[(2) - (3)]);
        ASTExpr *left = (ASTExpr *)(yyvsp[(1) - (3)]);
        ASTExpr *right = (ASTExpr *)(yyvsp[(3) - (3)]);
        e->setFirstOperand(left);
        e->setSecondOperand(right);
        (yyval)=e;;
    }
    break;
    
    case 113:
    
        /* Line 1455 of yacc.c  */
#line 452 "ffscript.ypp"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;
    
    case 114:
    
        /* Line 1455 of yacc.c  */
#line 455 "ffscript.ypp"
    {(yyval) = new ASTExprLShift((yylsp[(1) - (1)]));;}
    break;
    
    case 115:
    
        /* Line 1455 of yacc.c  */
#line 456 "ffscript.ypp"
    {(yyval) = new ASTExprRShift((yylsp[(1) - (1)]));;}
    break;
    
    case 116:
    
        /* Line 1455 of yacc.c  */
#line 459 "ffscript.ypp"
    {(yyval) = new ASTExprGT((yylsp[(1) - (1)]));;}
    break;
    
    case 117:
    
        /* Line 1455 of yacc.c  */
#line 460 "ffscript.ypp"
    {(yyval) = new ASTExprGE((yylsp[(1) - (1)]));;}
    break;
    
    case 118:
    
        /* Line 1455 of yacc.c  */
#line 461 "ffscript.ypp"
    {(yyval) = new ASTExprLT((yylsp[(1) - (1)]));;}
    break;
    
    case 119:
    
        /* Line 1455 of yacc.c  */
#line 462 "ffscript.ypp"
    {(yyval) = new ASTExprLE((yylsp[(1) - (1)]));;}
    break;
    
    case 120:
    
        /* Line 1455 of yacc.c  */
#line 463 "ffscript.ypp"
    {(yyval) = new ASTExprEQ((yylsp[(1) - (1)]));;}
    break;
    
    case 121:
    
        /* Line 1455 of yacc.c  */
#line 464 "ffscript.ypp"
    {(yyval) = new ASTExprNE((yylsp[(1) - (1)]));;}
    break;
    
    case 122:
    
        /* Line 1455 of yacc.c  */
#line 467 "ffscript.ypp"
    {
        ASTAddExpr *e = (ASTAddExpr *)(yyvsp[(2) - (3)]);
        ASTExpr *left = (ASTExpr *)(yyvsp[(1) - (3)]);
        ASTExpr *right = (ASTExpr *)(yyvsp[(3) - (3)]);
        e->setFirstOperand(left);
        e->setSecondOperand(right);
        (yyval)=e;;
    }
    break;
    
    case 123:
    
        /* Line 1455 of yacc.c  */
#line 473 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 124:
    
        /* Line 1455 of yacc.c  */
#line 476 "ffscript.ypp"
    {(yyval) = new ASTExprPlus((yylsp[(1) - (1)]));;}
    break;
    
    case 125:
    
        /* Line 1455 of yacc.c  */
#line 477 "ffscript.ypp"
    {(yyval) = new ASTExprMinus((yylsp[(1) - (1)]));;}
    break;
    
    case 126:
    
        /* Line 1455 of yacc.c  */
#line 480 "ffscript.ypp"
    {
        ASTMultExpr *e = (ASTMultExpr *)(yyvsp[(2) - (3)]);
        ASTExpr *left = (ASTExpr *)(yyvsp[(1) - (3)]);
        ASTExpr *right = (ASTExpr *)(yyvsp[(3) - (3)]);
        e->setFirstOperand(left);
        e->setSecondOperand(right);
        (yyval)=e;;
    }
    break;
    
    case 127:
    
        /* Line 1455 of yacc.c  */
#line 486 "ffscript.ypp"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;
    
    case 128:
    
        /* Line 1455 of yacc.c  */
#line 489 "ffscript.ypp"
    {(yyval) = new ASTExprTimes((yylsp[(1) - (1)]));;}
    break;
    
    case 129:
    
        /* Line 1455 of yacc.c  */
#line 490 "ffscript.ypp"
    {(yyval) = new ASTExprDivide((yylsp[(1) - (1)]));;}
    break;
    
    case 130:
    
        /* Line 1455 of yacc.c  */
#line 491 "ffscript.ypp"
    {(yyval) = new ASTExprModulo((yylsp[(1) - (1)]));;}
    break;
    
    case 131:
    
        /* Line 1455 of yacc.c  */
#line 494 "ffscript.ypp"
    {
        ASTUnaryExpr *e = new ASTExprNot((yylsp[(1) - (2)]));
        ASTExpr *op = (ASTExpr *)(yyvsp[(2) - (2)]);
        e->setOperand(op);
        (yyval)=e;;
    }
    break;
    
    case 132:
    
        /* Line 1455 of yacc.c  */
#line 498 "ffscript.ypp"
    {
        ASTUnaryExpr *e = new ASTExprNegate((yylsp[(1) - (2)]));
        ASTExpr *op = (ASTExpr *)(yyvsp[(2) - (2)]);
        e->setOperand(op);
        (yyval)=e;;
    }
    break;
    
    case 133:
    
        /* Line 1455 of yacc.c  */
#line 502 "ffscript.ypp"
    {
        ASTUnaryExpr *e = new ASTExprBitNot((yylsp[(1) - (2)]));
        ASTExpr *op = (ASTExpr *)(yyvsp[(2) - (2)]);
        e->setOperand(op);
        (yyval)=e;;
    }
    break;
    
    case 134:
    
        /* Line 1455 of yacc.c  */
#line 506 "ffscript.ypp"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;
    
    case 135:
    
        /* Line 1455 of yacc.c  */
#line 509 "ffscript.ypp"
    {(yyval)=(yyvsp[(2) - (3)]);;}
    break;
    
    case 136:
    
        /* Line 1455 of yacc.c  */
#line 510 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 137:
    
        /* Line 1455 of yacc.c  */
#line 511 "ffscript.ypp"
    {
        ASTUnaryExpr *e = new ASTExprIncrement((yylsp[(2) - (2)]));
        ASTExpr *op = (ASTExpr *)(yyvsp[(1) - (2)]);
        e->setOperand(op);
        (yyval)=e;;
    }
    break;
    
    case 138:
    
        /* Line 1455 of yacc.c  */
#line 515 "ffscript.ypp"
    {
        ASTUnaryExpr *e = new ASTExprPreIncrement((yylsp[(1) - (2)]));
        ASTExpr *op = (ASTExpr *)(yyvsp[(2) - (2)]);
        e->setOperand(op);
        (yyval)=e;;
    }
    break;
    
    case 139:
    
        /* Line 1455 of yacc.c  */
#line 519 "ffscript.ypp"
    {
        ASTUnaryExpr *e = new ASTExprDecrement((yylsp[(2) - (2)]));
        ASTExpr *op = (ASTExpr *)(yyvsp[(1) - (2)]);
        e->setOperand(op);
        (yyval)=e;;
    }
    break;
    
    case 140:
    
        /* Line 1455 of yacc.c  */
#line 523 "ffscript.ypp"
    {
        ASTUnaryExpr *e = new ASTExprPreDecrement((yylsp[(1) - (2)]));
        ASTExpr *op = (ASTExpr *)(yyvsp[(2) - (2)]);
        e->setOperand(op);
        (yyval)=e;;
    }
    break;
    
    case 141:
    
        /* Line 1455 of yacc.c  */
#line 527 "ffscript.ypp"
    {
        ASTFloat *val = (ASTFloat *)(yyvsp[(1) - (1)]);
        (yyval) = new ASTNumConstant(val,(yylsp[(1) - (1)]));;
    }
    break;
    
    case 142:
    
        /* Line 1455 of yacc.c  */
#line 529 "ffscript.ypp"
    {
        ASTString *as = (ASTString *)(yyvsp[(1) - (1)]);
        char val[15];
        sprintf(val, "%d", as->getValue().at(1));
        (yyval) = new ASTNumConstant(new ASTFloat(val,0,(yylsp[(1) - (1)])),(yylsp[(1) - (1)]));;
    }
    break;
    
    case 143:
    
        /* Line 1455 of yacc.c  */
#line 533 "ffscript.ypp"
    {(yyval) = (yyvsp[(1) - (1)]);;}
    break;
    
    case 144:
    
        /* Line 1455 of yacc.c  */
#line 534 "ffscript.ypp"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;
    
    case 145:
    
        /* Line 1455 of yacc.c  */
#line 537 "ffscript.ypp"
    {(yyval) = new ASTBoolConstant(true,(yylsp[(1) - (1)]));;}
    break;
    
    case 146:
    
        /* Line 1455 of yacc.c  */
#line 538 "ffscript.ypp"
    {(yyval) = new ASTBoolConstant(false,(yylsp[(1) - (1)]));;}
    break;
    
    case 147:
    
        /* Line 1455 of yacc.c  */
#line 541 "ffscript.ypp"
    {
        ASTFuncCall *fc = (ASTFuncCall *)(yyvsp[(3) - (4)]);
        ASTExpr *name = (ASTExpr *)(yyvsp[(1) - (4)]);
        fc->setName(name);
        (yyval)=fc;;
    }
    break;
    
    case 148:
    
        /* Line 1455 of yacc.c  */
#line 545 "ffscript.ypp"
    {
        ASTFuncCall *fc = new ASTFuncCall((yylsp[(1) - (3)]));
        ASTExpr *name = (ASTExpr *)(yyvsp[(1) - (3)]);
        fc->setName(name);
        (yyval)=fc;;
    }
    break;
    
    case 149:
    
        /* Line 1455 of yacc.c  */
#line 551 "ffscript.ypp"
    {
        ASTFuncCall *fc = (ASTFuncCall *)(yyvsp[(3) - (3)]);
        ASTExpr *e = (ASTExpr *)(yyvsp[(1) - (3)]);
        fc->addParam(e);
        (yyval) = fc;;
    }
    break;
    
    case 150:
    
        /* Line 1455 of yacc.c  */
#line 555 "ffscript.ypp"
    {
        ASTFuncCall *fc = new ASTFuncCall((yylsp[(1) - (1)]));
        ASTExpr *e = (ASTExpr *)(yyvsp[(1) - (1)]);
        fc->addParam(e);
        (yyval) = fc;;
    }
    break;
    
    case 151:
    
        /* Line 1455 of yacc.c  */
#line 561 "ffscript.ypp"
    {
        ASTStmt *prec = (ASTStmt *)(yyvsp[(3) - (9)]);
        ASTExpr *term = (ASTExpr *)(yyvsp[(5) - (9)]);
        ASTStmt *incr = (ASTExpr *)(yyvsp[(7) - (9)]);
        ASTStmt *stmt = (ASTStmt *)(yyvsp[(9) - (9)]);
        (yyval) = new ASTStmtFor(prec,term,incr,stmt,(yylsp[(1) - (9)]));;
    }
    break;
    
    case 152:
    
        /* Line 1455 of yacc.c  */
#line 568 "ffscript.ypp"
    {
        ASTExpr *cond = (ASTExpr *)(yyvsp[(3) - (5)]);
        ASTStmt *stmt = (ASTStmt *)(yyvsp[(5) - (5)]);
        (yyval) = new ASTStmtWhile(cond,stmt,(yylsp[(1) - (5)]));;
    }
    break;
    
    case 153:
    
        /* Line 1455 of yacc.c  */
#line 572 "ffscript.ypp"
    {
        ASTExpr *cond = (ASTExpr *)(yyvsp[(5) - (6)]);
        ASTStmt *stmt = (ASTStmt *)(yyvsp[(2) - (6)]);
        (yyval) = new ASTStmtDo(cond,stmt,(yylsp[(1) - (6)]));;
    }
    break;
    
    case 154:
    
        /* Line 1455 of yacc.c  */
#line 576 "ffscript.ypp"
    {
        ASTExpr *cond = (ASTExpr *)(yyvsp[(3) - (5)]);
        ASTStmt *stmt = (ASTStmt *)(yyvsp[(5) - (5)]);
        (yyval) = new ASTStmtIf(cond,stmt,(yylsp[(1) - (5)]));;
    }
    break;
    
    case 155:
    
        /* Line 1455 of yacc.c  */
#line 579 "ffscript.ypp"
    {
        ASTExpr *cond = (ASTExpr *)(yyvsp[(3) - (7)]);
        ASTStmt *ifstmt = (ASTStmt *)(yyvsp[(5) - (7)]);
        ASTStmt *elsestmt = (ASTStmt *)(yyvsp[(7) - (7)]);
        (yyval) = new ASTStmtIfElse(cond,ifstmt,elsestmt,(yylsp[(1) - (7)]));;
    }
    break;
    
    case 156:
    
        /* Line 1455 of yacc.c  */
#line 585 "ffscript.ypp"
    {(yyval) = new ASTStmtReturnVal((ASTExpr *)(yyvsp[(2) - (2)]),(yylsp[(1) - (2)]));;}
    break;
    
    case 157:
    
        /* Line 1455 of yacc.c  */
#line 586 "ffscript.ypp"
    {(yyval) = new ASTStmtReturn((yylsp[(1) - (1)]));;}
    break;
    
    
    
    /* Line 1455 of yacc.c  */
#line 3102 "y.tab.cpp"
    
    default:
        break;
    }
    
    YY_SYMBOL_PRINT("-> $$ =", yyr1[yyn], &yyval, &yyloc);
    
    YYPOPSTACK(yylen);
    yylen = 0;
    YY_STACK_PRINT(yyss, yyssp);
    
    *++yyvsp = yyval;
    *++yylsp = yyloc;
    
    /* Now `shift' the result of the reduction.  Determine what state
       that goes to, based on the state we popped back to and the rule
       number reduced by.  */
    
    yyn = yyr1[yyn];
    
    yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
    
    if(0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
        yystate = yytable[yystate];
    else
        yystate = yydefgoto[yyn - YYNTOKENS];
        
    goto yynewstate;
    
    
    /*------------------------------------.
    | yyerrlab -- here on detecting error |
    `------------------------------------*/
yyerrlab:

    /* If not already recovering from an error, report this error.  */
    if(!yyerrstatus)
    {
        ++yynerrs;
#if ! YYERROR_VERBOSE
        yyerror(YY_("syntax error"));
#else
        {
            YYSIZE_T yysize = yysyntax_error(0, yystate, yychar);
        
            if(yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
            {
                YYSIZE_T yyalloc = 2 * yysize;
        
                if(!(yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
                    yyalloc = YYSTACK_ALLOC_MAXIMUM;
        
                if(yymsg != yymsgbuf)
                    YYSTACK_FREE(yymsg);
        
                yymsg = (char *) YYSTACK_ALLOC(yyalloc);
        
                if(yymsg)
                    yymsg_alloc = yyalloc;
                else
                {
                    yymsg = yymsgbuf;
                    yymsg_alloc = sizeof yymsgbuf;
                }
            }
        
            if(0 < yysize && yysize <= yymsg_alloc)
            {
                (void) yysyntax_error(yymsg, yystate, yychar);
                yyerror(yymsg);
            }
            else
            {
                yyerror(YY_("syntax error"));
        
                if(yysize != 0)
                    goto yyexhaustedlab;
            }
        }
#endif
    }
    
    yyerror_range[0] = yylloc;
    
    if(yyerrstatus == 3)
    {
        /* If just tried and failed to reuse lookahead token after an
        error, discard it.  */
        
        if(yychar <= YYEOF)
        {
            /* Return failure if at end of input.  */
            if(yychar == YYEOF)
                YYABORT;
        }
        else
        {
            yydestruct("Error: discarding",
                       yytoken, &yylval, &yylloc);
            yychar = YYEMPTY;
        }
    }
    
    /* Else will try to reuse lookahead token after shifting the error
       token.  */
    goto yyerrlab1;
    
    
    /*---------------------------------------------------.
    | yyerrorlab -- error raised explicitly by YYERROR.  |
    `---------------------------------------------------*/
yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if(/*CONSTCOND*/ 0)
        goto yyerrorlab;
        
    yyerror_range[0] = yylsp[1-yylen];
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    YYPOPSTACK(yylen);
    yylen = 0;
    YY_STACK_PRINT(yyss, yyssp);
    yystate = *yyssp;
    goto yyerrlab1;
    
    
    /*-------------------------------------------------------------.
    | yyerrlab1 -- common code for both syntax error and YYERROR.  |
    `-------------------------------------------------------------*/
yyerrlab1:
    yyerrstatus = 3;	/* Each real token shifted decrements this.  */
    
    for(;;)
    {
        yyn = yypact[yystate];
        
        if(yyn != YYPACT_NINF)
        {
            yyn += YYTERROR;
            
            if(0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
                yyn = yytable[yyn];
                
                if(0 < yyn)
                    break;
            }
        }
        
        /* Pop the current state because it cannot handle the error token.  */
        if(yyssp == yyss)
            YYABORT;
            
        yyerror_range[0] = *yylsp;
        yydestruct("Error: popping",
                   yystos[yystate], yyvsp, yylsp);
        YYPOPSTACK(1);
        yystate = *yyssp;
        YY_STACK_PRINT(yyss, yyssp);
    }
    
    *++yyvsp = yylval;
    
    yyerror_range[1] = yylloc;
    /* Using YYLLOC is tempting, but would change the location of
       the lookahead.  YYLOC is available though.  */
    YYLLOC_DEFAULT(yyloc, (yyerror_range - 1), 2);
    *++yylsp = yyloc;
    
    /* Shift the error token.  */
    YY_SYMBOL_PRINT("Shifting", yystos[yyn], yyvsp, yylsp);
    
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
    
#if !defined(yyoverflow) || YYERROR_VERBOSE
    /*-------------------------------------------------.
    | yyexhaustedlab -- memory exhaustion comes here.  |
    `-------------------------------------------------*/
yyexhaustedlab:
    yyerror(YY_("memory exhausted"));
    yyresult = 2;
    /* Fall through.  */
#endif
    
yyreturn:

    if(yychar != YYEMPTY)
        yydestruct("Cleanup: discarding lookahead",
                   yytoken, &yylval, &yylloc);
                   
    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    YYPOPSTACK(yylen);
    YY_STACK_PRINT(yyss, yyssp);
    
    while(yyssp != yyss)
    {
        yydestruct("Cleanup: popping",
                   yystos[*yyssp], yyvsp, yylsp);
        YYPOPSTACK(1);
    }
    
#ifndef yyoverflow
    
    if(yyss != yyssa)
        YYSTACK_FREE(yyss);
        
#endif
#if YYERROR_VERBOSE
        
    if(yymsg != yymsgbuf)
        YYSTACK_FREE(yymsg);
        
#endif
    /* Make sure YYID is used.  */
    return YYID(yyresult);
}



/* Line 1675 of yacc.c  */
#line 589 "ffscript.ypp"


/*        programs */

void yyerror(const char *s)
{
    char temp[512];
    snprintf(temp, 512, "line %d: %s, on token %s", yylineno, s, yytext);
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


