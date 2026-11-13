// @ts-check
const { defineConfig } = require('eslint/config');
const eslint = require('@eslint/js');
const stylistic = require('@stylistic/eslint-plugin');
const tseslint = require('typescript-eslint');

module.exports = defineConfig(
	{
		ignores: ['**/node_modules/', 'client/out/', 'server/out/'],
	},
	{
		files: ['**/*.ts', '**/*.tsx'],
		extends: [eslint.configs.recommended, tseslint.configs.recommended],
		plugins: {
			'@stylistic': stylistic,
		},
		rules: {
			'@stylistic/semi': ['error', 'always'],
			'@typescript-eslint/no-unused-vars': 'off',
			'@typescript-eslint/no-explicit-any': 'off',
			'@typescript-eslint/explicit-module-boundary-types': 'off',
			'@typescript-eslint/no-non-null-assertion': 'off',
		},
	},
);
