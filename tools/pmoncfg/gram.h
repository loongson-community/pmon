/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
     AND = 258,
     AT = 259,
     ATTACH = 260,
     BUILD = 261,
     COMPILE_WITH = 262,
     CONFIG = 263,
     DEFINE = 264,
     DEFOPT = 265,
     DEVICE = 266,
     DISABLE = 267,
     DUMPS = 268,
     ENDFILE = 269,
     XFILE = 270,
     XOBJECT = 271,
     FLAGS = 272,
     INCLUDE = 273,
     XMACHINE = 274,
     MAJOR = 275,
     MAKEOPTIONS = 276,
     MINOR = 277,
     ON = 278,
     OPTIONS = 279,
     SELECT = 280,
     UNSELECT = 281,
     PSEUDO_DEVICE = 282,
     ROOT = 283,
     SOURCE = 284,
     WITH = 285,
     NEEDS_COUNT = 286,
     NEEDS_FLAG = 287,
     RMOPTIONS = 288,
     ENABLE = 289,
     NUMBER = 290,
     PATHNAME = 291,
     WORD = 292,
     EMPTY = 293
   };
#endif
/* Tokens.  */
#define AND 258
#define AT 259
#define ATTACH 260
#define BUILD 261
#define COMPILE_WITH 262
#define CONFIG 263
#define DEFINE 264
#define DEFOPT 265
#define DEVICE 266
#define DISABLE 267
#define DUMPS 268
#define ENDFILE 269
#define XFILE 270
#define XOBJECT 271
#define FLAGS 272
#define INCLUDE 273
#define XMACHINE 274
#define MAJOR 275
#define MAKEOPTIONS 276
#define MINOR 277
#define ON 278
#define OPTIONS 279
#define SELECT 280
#define UNSELECT 281
#define PSEUDO_DEVICE 282
#define ROOT 283
#define SOURCE 284
#define WITH 285
#define NEEDS_COUNT 286
#define NEEDS_FLAG 287
#define RMOPTIONS 288
#define ENABLE 289
#define NUMBER 290
#define PATHNAME 291
#define WORD 292
#define EMPTY 293




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 94 "gram.y"
{
	struct	attr *attr;
	struct	devbase *devb;
	struct	deva *deva;
	struct	nvlist *list;
	const char *str;
	int	val;
}
/* Line 1529 of yacc.c.  */
#line 134 "gram.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

