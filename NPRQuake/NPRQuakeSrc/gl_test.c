/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "quakedef.h"

#ifdef GLTEST

typedef struct
{
	plane_t	*plane;
	vec3_t	origin;
	vec3_t	normal;
	vec3_t	up;
	vec3_t	right;
	vec3_t	reflect;
	float	length;
} puff_t;

#define	MAX_PUFFS	64

puff_t	puffs[MAX_PUFFS];


void Test_Init (void)
{
}



plane_t	junk;
plane_t	*HitPlane (vec3_t start, vec3_t end)
{
	trace_t		trace;

// fill in a default trace
	memset (&trace, 0, sizeof(trace_t));
	trace.fraction = 1;
	trace.allsolid = true;
	VectorCopy (end, trace.endpos);

	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);

	junk = trace.plane;
	return &junk;
}

void Test_Spawn (vec3_t origin)
{
	int		i;
	puff_t	*p;
	vec3_t	temp;
	vec3_t	normal;
	vec3_t	incoming;
	plane_t	*plane;
	float	d;

	for (i=0,p=puffs ; i<MAX_PUFFS ; i++,p++)
	{
		if (p->length <= 0)
			break;
	}
	if (i == MAX_PUFFS)
		return;

	VectorSubtract (r_refdef.vieworg, origin, incoming);
	VectorSubtract (origin, incoming, temp);
	plane = HitPlane (r_refdef.vieworg, temp);

	VectorNormalize (incoming);
	d = DotProduct (incoming, plane->normal);
	VectorSubtract (vec3_origin, incoming, p->reflect);
	VectorMA (p->reflect, d*2, plane->normal, p->reflect);

	VectorCopy (origin, p->origin);
	VectorCopy (plane->normal, p->normal);

	CrossProduct (incoming, p->normal, p->up);

	CrossProduct (p->up, p->normal, p->right);

	p->length = 8;
}



void Test_Draw (void)
{
	int		i;
	puff_t	*p;

	for (i=0, p=puffs ; i<MAX_PUFFS ; i++,p++)
	{
		if (p->length > 0)
			DrawPuff (p);
	}
}

#endif
