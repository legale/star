/* @(#)fpipe.c	1.9 00/12/03 Copyright 1986, 1995 J. Schilling */
/*
 *	Copyright (c) 1986, 1995 J. Schilling
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

#include "io.h"

EXPORT int
fpipe (pipef)
	FILE *pipef[];
{
	int filedes[2];

	if (pipe(filedes) < 0)
		return 0;

	if ((pipef[0] = _fcons((FILE *)0,
				filedes[0], FI_READ|FI_CLOSE)) != (FILE *)0) {
		if ((pipef[1] = _fcons((FILE *)0,
				filedes[1], FI_WRITE|FI_CLOSE)) != (FILE *)0) {
			return (int)pipef[0];
		}
		fclose (pipef[0]);
	}
	close (filedes[1]);
	return 0;
}
