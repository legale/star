/* @(#)fileseek.c	1.5 96/05/09 Copyright 1986 J. Schilling */
/*
 *	Copyright (c) 1986 J. Schilling
 */
/* This program is free software; you can redistribute it and/or modify
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

#include <stdio.h>
#include "io.h"

static	char	_seekerr[]	= "file_seek_err";

long fileseek (f, pos)
	register FILE	*f;
	long	pos;
{
	long	ret;

	down(f);
	ret = fseek (f, pos, 0);
	if (ret < 0 && !(my_flag(f) & _IONORAISE))
		raisecond(_seekerr, 0L);
	return ret;
}

