/* @(#)patmatch.h	1.4 96/02/04 Copyright 1985 J. Schilling */

#ifndef	_PATMATCH_H
#define	_PATMATCH_H
/*
 *	Definitions for the pattern matching functions.
 *
 *	Copyright (c) 1985,1995 J. Schilling
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 *	The pattern matching functions are based on the algorithm
 *	presented by Martin Richards in:
 *
 *	"A Compact Function for Regular Expression Pattern Matching", 
 *	Software-Practice and Experience, Vol. 9, 527-534 (1979)
 *
 *	Several changes have been made to the original source which has been
 *	written in BCPL:
 *
 *	'/'	is replaced by	'!'		(to allow UNIX filenames)
 *	'(',')' are replaced by	'{', '}'
 *	'\''	is replaced by	'\\'		(UNIX compatible quote)
 *
 *	Character classes have been added to allow "[<character list>]"
 *	to be used.
 *	Start of line '^' and end of line '$' have been added.
 *
 *	Any number in the following comment is zero or more occurrencies
 */
#define	ALT	'!'	/* Alternation in match i.e. this!that!the_other */
#define	REP	'#'	/* Any number of occurrences of the following expr */
#define	NIL	'%'	/* Empty string (exactly nothing) */
#define	STAR	'*'	/* Any number of any character (equivalent of #?) */
#define	ANY	'?'	/* Any one character */
#define	QUOTE	'\\'	/* Quotes the next character */
#define	LBRACK	'{'	/* Begin of precedence grouping */
#define	RBRACK	'}'	/* End of precedence grouping */
#define	LCLASS	'['	/* Begin of character set */
#define	RCLASS	']'	/* End of character set */
#define	NOT	'^'	/* If first in set: invert set content */
#define	RANGE	'-'	/* Range notation in sets */
#define	START	'^'	/* Begin of a line */
#define	END	'$'	/* End of a line */

#define	MAXPAT	128	/* Maximum length of pattern */

#ifdef	__STDC__

extern	int		patcompile(const unsigned char *, int, int *);
extern	unsigned char	*patmatch(const unsigned char *, const int *,
				  const unsigned char *, int, int, int);

#else

extern	int		patcompile();
extern	unsigned char	*patmatch();

#endif

#endif	/* _PATMATCH_H */
