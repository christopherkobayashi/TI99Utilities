#ifndef	LIB_H
#define	LIB_H

#include <config.h>
#include <stddef.h>

/* 
 * Copyright (c) 1998 peter memishian (meem), meem@gnu.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 */

void	*xmalloc(size_t);
char	*xstrdup(const char *);

#ifndef	HAVE_ALLOCA
void	*alloca(size_t);
#endif

#ifndef	HAVE_STRCASECMP
int	strcasecmp(const char *, const char *);
#endif

#ifndef	HAVE_STRDUP
char	*strdup(const char *);
#endif

#ifndef	HAVE_STRSTR
char	*strstr(const char *, const char *);
#endif

#endif	/* LIB_H */
