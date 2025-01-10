/*
Language: ZScript
Category: custom
Website: https://docs.zquestclassic.com/
*/

function regex_escape(value) {
	return value.replace(/[-/\\^$*+?.()|[\]{}]/g, '\\$&');
}

/** @type LanguageFn */
export default function(hljs) {
	
	const regex = hljs.regex;
	
	const OPT_WHITESPACE_RE = '\\s*';
	const SOME_WHITESPACE_RE = '\\s+';
	const IDENTIFIER_DELIMITER_RE = '(?:\\.|::)'
	const IDENTIFIER_RE = hljs.UNDERSCORE_IDENT_RE;
	const IDENTIFIER_LIST_RE = '(?:(?:' + IDENTIFIER_RE + ')?' + IDENTIFIER_DELIMITER_RE + ')?'
		+ '(?:' + IDENTIFIER_RE + IDENTIFIER_DELIMITER_RE + ')*'
		+ IDENTIFIER_RE;
	const TEMPLATE_ARGUMENT_RE = '<[^<>]+>';
	
	const FUNCTION_KEYWORDS = [
		'static', 'inline', 'constexpr', 'internal'
	];
	const FUNCTION_KEYWORDS_RE = '(?:' + FUNCTION_KEYWORDS.join('|') + ')';
	
	const TYPE_RE = '(?:const' + SOME_WHITESPACE_RE + ')?' + IDENTIFIER_LIST_RE;
	
	const CHARACTER_ESCAPES = '\\\\(x[0-9A-Fa-f]{2}|\\S)';
	const STRINGS = {
		scope: 'string',
		variants: [
			{
				begin: '"',
				end: '"',
				relevance: 0,
				illegal: '\\n',
				contains: [ hljs.BACKSLASH_ESCAPE ]
			},
			{
				begin: '\'(' + CHARACTER_ESCAPES + '|.)',
				end: '\'',
				relevance: 0,
				illegal: '.'
			}
		]
	};
	
	const INT_LITERAL = "\\b(?:"
		+ "0x[0-9a-fA-F]+" // hex-prefixed value
		+ "|0b[0-1]+" // binary-prefixed value
		+ "|0o[0-7]+" // octal-prefixed value
		+ "|[0-1]+b" // binary-suffixed value
		+ "|[0-7]+o" // octal-suffixed value
		+ "|[0-9]*\\.?[0-9]+" // Standard fixed point value
		+ ")\\b";
	
	const LONG_LITERAL = "\\b(?:"
		+ "0x[0-9a-fA-F]+L" // hex-prefixed value
		+ "|0b[0-1]+L" // binary-prefixed value
		+ "|0o[0-7]+L" // octal-prefixed value
		+ "|[0-1]+(?:bL|Lb)" // binary-suffixed value
		+ "|[0-7]+(?:oL|Lo)" // octal-suffixed value
		+ "|[0-9]+L" // long-suffixed value
		+ ")\\b";
	
	const NUMBERS = {
		scope: 'number',
		variants: [
			{
				match: INT_LITERAL
			},
			{ 
				match: LONG_LITERAL
			}
		],
		relevance: 0
	};
	
	const RESERVED_KEYWORDS = [
		'always',
		'and',
		'and_eq',
		'and_equal',
		'asm',
		'appx_eq',
		'appx_equal',
		'bitand',
		'bitnot',
		'bitor',
		'bitxor',
		'break',
		'case',
		'catch',
		'class',
		'compl',
		'constexpr',
		'continue',
		'default',
		'delete',
		'do',
		'else',
		'enum',
		'equals',
		'false',
		'for',
		'if',
		'in',
		'import',
		'inline',
		'internal',
		'loop',
		'namespace',
		'new',
		'not',
		'not_eq',
		'not_equal',
		'or',
		'or_eq',
		'or_equal',
		'repeat',
		'return',
		'script',
		'switch',
		'this',
		'true',
		'try',
		'typedef',
		'unless',
		'until',
		'using',
		'while',
		'xor',
		'xor_eq',
		'xor_equal',
		'zasm',
		'CONST_ASSERT',
		'IS_INCLUDED',
		'OPTION_VALUE'
	];
	
	const RESERVED_TYPES = [
		'auto',
		'bool',
		'char32',
		'float',
		'int',
		'long',
		'untyped',
		'rgb',
		'void',
		'const',
		'static'
	];
	
	const STANDARD_OPERATORS = [
		'^^^',
		'...', '=..=', '=..', '..=', '..',
		'++', '--', '->',
		
		':=', '+=', '-=', '*=', '/=', '%=', '<<=', '>>=',
		'&=', '|=', '^=', '~=', '&&=', '||=',
		
		'<<', '>>', '<=', '>=', '==', '!=',
		'<>', '~~', '^^', '&&', '||', 
		
		'-', '!', '~', '*', '/', '%', '+',
		'<', '>', '&', '^', '|', '?', '='
	];
	const KEYWORD_OPERATORS = [
		'not', 'bitnot', 'compl', 'equals', 'not_eq', 'not_equal',
		'appx_eq', 'appx_equal', 'xor', 'bitand', 'bitxor',
		'bitor', 'and', 'or', 'delete', 'and_eq', 'and_equal',
		'or_eq', 'or_equal', 'xor_eq', 'xor_equal'
	];
	
	const KEYWORD_OPERATORS_SCOPE = {
		scope: 'operator',
		match: '\\b(?:' + KEYWORD_OPERATORS.join('|') + ')\\b'
	};
	const SYMBOL_OPERATORS_SCOPE = {
		scope: 'operator',
		match: '(?:' + STANDARD_OPERATORS.map(regex_escape).join('|') + ')'
	};
	
	const BINDING_TYPES = [
		// bindings types for instance classes
		'bitmap',
		'bottledata',
		'bottleshopdata',
		'combodata',
		'directory',
		'dmapdata',
		'dropsetdata',
		'eweapon',
		'ffc',
		'file',
		'genericdata',
		'itemdata',
		'itemsprite',
		'lweapon',
		'messagedata',
		'mapdata',
		'npc',
		'npcdata',
		'paldata',
		'portal',
		'randgen',
		'savedportal',
		'shopdata',
		'sprite',
		'spritedata',
		'stack',
		'subscreendata',
		'subscreenpage',
		'subscreenwidget',
		'websocket',
		// binding types for global classes
		// 'Audio',
		// 'Debug',
		// 'FileSystem',
		// 'Game',
		// 'Graphics',
		// 'hero',
		// 'Input',
		// 'Region',
		// 'screendata',
		// 'Text',
		// 'ZInfo',
		// binding enum types
		'Direction',
		'EnemyPattern',
		'CounterIndex',
		'SwitchEffect',
		'WarpType',
		'WeaponFlagIndex',
		'WeaponSpriteIndex',
		'ShadowType',
		'DrawStyle',
		'TransformationType',
		'ComboType',
		'TileWarpIndex',
		'SideWarpIndex',
		'WarpIndex',
		//
		'GravityIndex',
		'ScrollingIndex',
		'MiscSpriteIndex',
		'MiscSfx',
		'GenericIndex',
		'MouseCursor',
		//
		'HeroSliding',
		//
		'ItemClass',
		//
		'NpcType',
		'NpcDefenseIndex',
		'NpcDefenseType',
		'NpcWeapon',
		'ItemSet',
		'BossPal',
		'NpcMoveFlagIndex',
		'NpcShieldIndex',
		'NpcFade',
		//
		'QR',
		//
		'WipeEffect',
		//
		'WebsocketState',
		'WebsocketType'
	];
	
	const GLOBAL_POINTERS = [
		'Audio', 'Debug', 'FileSystem', 'Game', 'Graphics', 'Hero',
		'Input', 'Link', 'Module', 'Player', 'RandGen', 'Region',
		'Screen', 'Text', 'ZInfo'
	];
	
	const STD_TYPES = [
		// std.zh typedefs
		'define',
		'DEFINE',
		'DEFINEF',
		'DEFINEL',
		'DEFINEB',
		'DEFINEU',
		'CONFIG',
		'CONFIGF',
		'CONFIGL',
		'CONFIGB',
		'CONFIGU',
		// std.zh enums
		'WeaponDeadState',
		'roomtypes',
		'catchalltypes',
		'ScreenState',
		'HeroAction',
		'itemslots',
		'DmapType',
		'dmapgridpositions',
		'linkspritetype',
		'mapscreenflags',
		'HeroMoveFlag',
		'ItemspriteMoveFlag',
		'WeaponMoveFlag',
		'MoveFlag',
		'NPCWalkType',
		'NPCMoveStatus',
		'ditherType',
		'BottleFlag',
		'ComboGenFlag',
		'InteractButton',
		'ItemButton',
		'ComboTrigFlag',
		'scr_timing',
		'SubscreenWidgetType',
		'SubscreenPageMode'
	];
	
	const EXPECTED_TYPES = RESERVED_TYPES.concat(BINDING_TYPES, STD_TYPES);
	
	const WAIT_FUNCTIONS = [
		'Waitframe', 'Waitframes', 'Waitdraw', 'WaitTo', 'WaitEvent'
	];
	
	const FUNCTION_HINTS = [
		'Quit',
		'QuitNoKill',
		'Waitframe',
		'Waitframes',
		'Waitdraw',
		'WaitTo',
		'WaitEvent',
		'Trace',
		'TraceB',
		'TraceS',
		'TraceNL',
		'ClearTrace',
		'TraceToBase',
		'Sin',
		'Cos',
		'Tan',
		'ArcTan',
		'ArcSin',
		'ArcCos',
		'RadianSin',
		'RadianCos',
		'RadianTan',
		'Pow',
		'LPow',
		'InvPow',
		'Factorial',
		'Abs',
		'Log10',
		'Ln',
		'Sqrt',
		'CopyTile',
		'SwapTile',
		'ClearTile',
		'SizeOfArray',
		'ResizeArray',
		'OwnArray',
		'DestroyArray',
		'OwnObject',
		'GlobalObject',
		'OverlayTile',
		'Floor',
		'Ceiling',
		'Truncate',
		'Round',
		'RoundAway',
		'GetSystemTime',
		'Distance',
		'LongDistance',
		'strcmp',
		'strncmp',
		'stricmp',
		'strnicmp',
		'strcpy',
		'itoacat',
		'ArrayCopy',
		'strlen',
		'atoi',
		'atol',
		'ilen',
		'utol',
		'ltou',
		'convcase',
		'itoa',
		'xtoa',
		'xtoi',
		'SaveSRAM',
		'LoadSRAM',
		'strcat',
		'strchr',
		'strcspn',
		'strspn',
		'strstr',
		'strrchr',
		'IsValidArray',
		'DegtoRad',
		'RadtoDeg',
		'DegToRad',
		'RadToDeg',
		'WrapRadians',
		'WrapDegrees',
		'printf',
		'sprintf',
		'printfa',
		'sprintfa',
		'Max',
		'Min',
		'Choose',
		'ArrayPushBack',
		'ArrayPushBack',
		'ArrayPushAt',
		'ArrayPopBack',
		'ArrayPopFront',
		'ArrayPopAt',
		'Rand',
		'SRand',
		'ComboAt',
		'RefCount',
		'GC',
	];
	
	const HASHMODE = {
		scope: 'meta',
		begin: /#\s*[a-zA-Z_][a-zA-Z_0-9]*\b/,
		end: /$/,
		keywords: { 'keyword.meta':
			['option', 'ignore error', 'IGNORE ERROR', 'ignore warning',
			'IGNORE WARNING', 'define', 'include', 'includepath',
			'includeif', 'inherit', 'on', 'off', 'error', 'warn'] },
		contains: [
			{ // String without escape characters
				scope: 'string',
					variants: [
					{
						begin: '"',
						end: '"',
						relevance: 0,
						illegal: '\\n'
					}
				]
			},
			KEYWORD_OPERATORS_SCOPE,
			SYMBOL_OPERATORS_SCOPE,
			NUMBERS
		]
	};
	
	const ANNOTATION = {
		scope: 'meta.annotation',
		begin: '@' + OPT_WHITESPACE_RE + IDENTIFIER_RE + OPT_WHITESPACE_RE + '\\(',
		end: '\\)',
		contains: [
			STRINGS,
			NUMBERS
		]
	};
	
	const LITERALS = [
		'NULL',
		'null',
		'Null',
		'false',
		'true',
		'INVALID_COLOR'
	];
	
	const ZSCRIPT_KEYWORDS = {
		type: EXPECTED_TYPES,
		'keyword.literal': LITERALS,
		'keyword.global': GLOBAL_POINTERS,
		'keyword.wait': WAIT_FUNCTIONS,
		keyword: RESERVED_KEYWORDS
	};
	
	const EXPRESSION_CONTAINS = [
		hljs.C_LINE_COMMENT_MODE,
		hljs.C_BLOCK_COMMENT_MODE,
		KEYWORD_OPERATORS_SCOPE,
		SYMBOL_OPERATORS_SCOPE,
		NUMBERS,
		STRINGS
	];
	
	const USING_STATEMENT = {
		match: [
			'(?:always' + SOME_WHITESPACE_RE + ')?using',
			SOME_WHITESPACE_RE,
			'namespace',
			SOME_WHITESPACE_RE,
			IDENTIFIER_LIST_RE
		],
		scope: {
			1: 'keyword',
			3: 'keyword',
			5: 'title.namespace'
		}
	};
	
	const PAREN_MATCHER = {
		begin: /\(/,
		end: /\)/,
		keywords: ZSCRIPT_KEYWORDS,
		relevance: 0,
		contains: [
			'self',
			hljs.C_LINE_COMMENT_MODE,
			hljs.C_BLOCK_COMMENT_MODE,
			STRINGS,
			KEYWORD_OPERATORS_SCOPE,
			SYMBOL_OPERATORS_SCOPE,
			NUMBERS
		]
	};
	const BRACE_MATCHER = {
		begin: /{/,
		end: /}/,
		keywords: ZSCRIPT_KEYWORDS,
		relevance: 0,
		contains: [
			'self',
			hljs.C_LINE_COMMENT_MODE,
			hljs.C_BLOCK_COMMENT_MODE,
			STRINGS,
			KEYWORD_OPERATORS_SCOPE,
			SYMBOL_OPERATORS_SCOPE,
			NUMBERS
		]
	};
	
	const FUNC_POSTHEADER = {
		end: /;/,
		excludeEnd: true,
		contains: [
			{ // Prototype w/ default
				match: [
					':',
					OPT_WHITESPACE_RE,
					'default',
					SOME_WHITESPACE_RE
				],
				scope: {
					3: 'keyword'
				},
				starts: {
					end: /;/,
					excludeEnd: true,
					contains: [
						PAREN_MATCHER,
						BRACE_MATCHER
					],
					endsParent: true
				}
			},
			{ // Body
				begin: /{/,
				end: /}/,
				scope: 'function.body',
				keywords: ZSCRIPT_KEYWORDS,
				relevance: 0,
				endsParent: true,
				contains: [
					PAREN_MATCHER,
					BRACE_MATCHER,
					USING_STATEMENT,
					hljs.C_LINE_COMMENT_MODE,
					hljs.C_BLOCK_COMMENT_MODE,
					STRINGS,
					KEYWORD_OPERATORS_SCOPE,
					SYMBOL_OPERATORS_SCOPE,
					NUMBERS
				]
			}
		]
	};
	
	const FUNC_PARAMS = {
		scope: 'params',
		begin: /\(/,
		end: /\)/,
		excludeBegin: true,
		excludeEnd: true,
		keywords: ZSCRIPT_KEYWORDS,
		relevance: 0,
		endsParent: true,
		contains: [
			{
				match: [
					IDENTIFIER_LIST_RE,
					SOME_WHITESPACE_RE,
					IDENTIFIER_RE,
					regex.optional(SOME_WHITESPACE_RE),
					'=?',
					regex.optional(SOME_WHITESPACE_RE)
				],
				scope: {
					1: 'type.param',
					3: 'title.param',
					5: 'operator'
				},
				keywords: ZSCRIPT_KEYWORDS,
				starts: { // opt param initializer
					scope: 'optional_init',
					end: '(?=,|\\))',
					contains: [
						BRACE_MATCHER,
						PAREN_MATCHER
					].concat(EXPRESSION_CONTAINS)
				}
			},
			hljs.C_LINE_COMMENT_MODE,
			hljs.C_BLOCK_COMMENT_MODE,
			STRINGS,
			KEYWORD_OPERATORS_SCOPE,
			SYMBOL_OPERATORS_SCOPE,
			NUMBERS,
			PAREN_MATCHER,
		]
	};
	const FUNC_TEMPLATING = {
		scope: 'templating',
		begin: /</,
		end: />/,
		keywords: ZSCRIPT_KEYWORDS,
		contains: [
			{
				scope: 'type.template',
				match: IDENTIFIER_RE
			}
		]
	};
	
	const FUNC_HEADER = {
		end: hljs.MATCH_NOTHING_RE,
		keywords: ZSCRIPT_KEYWORDS,
		contains: [
			FUNC_TEMPLATING,
			FUNC_PARAMS,
			hljs.C_LINE_COMMENT_MODE,
			hljs.C_BLOCK_COMMENT_MODE
		],
		starts: FUNC_POSTHEADER
	};
	
	const RUN_FUNC_DECLARATION = {
		match: [
			'(?:' + FUNCTION_KEYWORDS_RE + SOME_WHITESPACE_RE + ')*', // keywords
			'void' + SOME_WHITESPACE_RE + 'run',
			'(?=\\s*(?:<[^>]*>)?\\s*\\([^\\)]*\\))'
		],
		scope: {
			2: 'keyword.runfunc'
		},
		keywords: ZSCRIPT_KEYWORDS,
		starts: FUNC_HEADER
	};
	
	const FUNCTION_DECLARATION = {
		match: [
			'(?:' + FUNCTION_KEYWORDS_RE + SOME_WHITESPACE_RE + ')*', // keywords
			TYPE_RE, // return type
			SOME_WHITESPACE_RE,
			IDENTIFIER_LIST_RE, // func name
			'(?=\\s*(?:<[^>]*>)?\\s*\\([^\\)]*\\))'
		],
		scope: {
			2: 'type.return',
			4: 'title.function',
		},
		keywords: ZSCRIPT_KEYWORDS,
		starts: FUNC_HEADER
	};
	
	const SCRIPT_DECLARATION = {
		match: [
			IDENTIFIER_LIST_RE,
			SOME_WHITESPACE_RE,
			'script',
			SOME_WHITESPACE_RE,
			IDENTIFIER_LIST_RE
		],
		scope: {
			1: 'type.script',
			3: 'keyword',
			5: 'title.script',
		}
	};
	
	const ENUM_DECLARATIONS = [
		{ // ex. 'enum = long'
			match: [
				/\b(?:enum)/,
				/\s*=\s*/,
				IDENTIFIER_LIST_RE
			],
			scope: {
				1: 'keyword',
				3: 'type'
			}
		},
		{ // ex. 'enum Direction'
			match: [
				/\b(?:enum)/,
				SOME_WHITESPACE_RE,
				IDENTIFIER_LIST_RE
			],
			scope: {
				1: 'keyword',
				3: 'type.enum'
			}
		},
	];
	
	const CLASS_DECLARATION = {
		match: [
			/\b(?:class)/,
			SOME_WHITESPACE_RE,
			IDENTIFIER_LIST_RE
		],
		scope: {
			1: 'keyword',
			3: 'type.class'
		}
	};
	
	const NAMESPACE_DECLARATION = {
		match: [
			/\b(?:namespace)/,
			SOME_WHITESPACE_RE,
			IDENTIFIER_LIST_RE
		],
		scope: {
			1: 'keyword',
			3: 'title.namespace'
		}
	};
	
	return {
		name: 'ZScript',
		aliases: [
			'zs',
			'zh'
		],
		keywords: ZSCRIPT_KEYWORDS,
		illegal: '</',
		contains: [].concat(
			RUN_FUNC_DECLARATION,
			FUNCTION_DECLARATION,
			EXPRESSION_CONTAINS, // list
			HASHMODE,
			ANNOTATION,
			USING_STATEMENT,
			NAMESPACE_DECLARATION,
			CLASS_DECLARATION,
			SCRIPT_DECLARATION,
			ENUM_DECLARATIONS
		)
	};
}

