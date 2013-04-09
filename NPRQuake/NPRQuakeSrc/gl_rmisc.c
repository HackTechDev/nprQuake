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
// r_misc.c

#include "quakedef.h"
#ifndef _WIN32     //
#include <dlfcn.h> //JG
#endif             //


///////////////////////////////////////////////////////
// all this was added for the DLL rendering -- Alex


// 25 functions

void (*dr_Init)();
void (*dr_Shutdown)();

void (*dr_Set_Cvar_RegisterVariable)( void (*f)( cvar_t * ) );
void (*dr_Set_Cvar_UnRegisterVariable)( void (*f)( cvar_t * ) );
void (*dr_Set_ConPrintf)( void (*f)( char * fmt, ... ) );
void (*dr_Set_VectorMA)( void (*f)( vec3_t, float, vec3_t, vec3_t ) );
void (*dr_Set_GL_Bind)( void (*f)( int ) );
void (*dr_Set_GL_DisableMultitexture)( void (*f)() );
void (*dr_Set_GL_EnableMultitexture)( void (*f)() );
void (*dr_Set_AngleVectors)( void (*f)( vec3_t, vec3_t, vec3_t, vec3_t ) );
void (*dr_Set_R_TextureAnimation)( texture_t * (*f)( texture_t * ) );
void (*dr_Set_VectorScale)( void (*f)( vec3_t, vec_t, vec3_t ) );


void (*GL_SelectTexture )(GLenum target, qboolean bl_mtexable, 
                       lpSelTexFUNC qglSelectTextureSGIS, int currenttexture,
                       int * cnttextures);

void (*R_DrawSpriteModel )(entity_t *e, entity_t *currententity,
                        double cltime, vec3_t vup, vec3_t vright);

void (*GL_DrawAliasFrame )(aliashdr_t *paliashdr, int posenum, 
                        float * shadedots, float shadelight, entity_t	*currententity );
//void (*GL_DrawAliasFrame )(aliashdr_t *paliashdr, int posenum, 
//                        float * shadedots, float shadelight);


void (*GL_DrawAliasShadow )(aliashdr_t *paliashdr, entity_t * currententity,
                         int posenum, vec3_t shadevector,
                         vec3_t lightspot);

void (*R_PolyBlend )(float * v_blend, cvar_t * gl_polyblend);
void (*R_DrawSequentialPoly )(msurface_t *s, int lightmap_textures,
                           qboolean * lightmap_modified, 
                           glRect_t * lightmap_rectchange, byte * lightmaps,
                           int lightmap_bytes, int solidskytexture, 
                           float * speedscale, int alphaskytexture, 
                           qboolean gl_mtexable, int currenttexture,
                           int * cnttextures, double realtime, 
                           lpMTexFUNC qglMTexCoord2fSGIS, int gl_lightmap_format,
                           lpSelTexFUNC qglSelectTextureSGIS, vec3_t r_origin );
void (*DrawGLWaterPoly )(glpoly_t *p, double realtime);
void (*DrawGLWaterPolyLightmap )(glpoly_t *p, double realtime);
void (*DrawGLPoly )(glpoly_t *p);
void (*R_BlendLightmaps )( glpoly_t ** lightmap_polys, int lightmap_textures,
                       qboolean * lightmap_modified, glRect_t * lightmap_rectchange,
                       byte * lightmaps, int lightmap_bytes, int gl_lightmap_format,
                       cvar_t * r_lightmap, cvar_t * gl_texsort, cvar_t * r_fullbright,
                       double realtime );
void (*R_DrawParticles )(particle_t ** active_particles, particle_t ** free_particles,
                      int * ramp1, int * ramp2, int * ramp3, cvar_t * sv_gravity,
                      double cloldtime, double cltime, int particletexture,
                      vec3_t vright, vec3_t vup, unsigned int * d_8to24table,
                      vec3_t vpn, vec3_t r_origin);
void (*EmitWaterPolys )(msurface_t *fa, double realtime);
void (*EmitSkyPolys )(msurface_t *fa, float speedscale, vec3_t r_origin);


#ifdef _WIN32 //JG
HMODULE holdlib = NULL;
HMODULE hnewlib = NULL;
#else
void *holdlib = NULL;
void *hnewlib = NULL;
#endif

int R_dr_LoadRenderer_helper( const char * lib );
void R_dr_LoadRenderer_f( void );



////////////////////////////////////////////////////////





/*
==================
R_InitTextures
==================
*/
void	R_InitTextures (void)
{
	int		x,y, m;
	byte	*dest;

// create a simple checkerboard texture for the default
	r_notexture_mip = Hunk_AllocName (sizeof(texture_t) + 16*16+8*8+4*4+2*2, "notexture");
	
	r_notexture_mip->width = r_notexture_mip->height = 16;
	r_notexture_mip->offsets[0] = sizeof(texture_t);
	r_notexture_mip->offsets[1] = r_notexture_mip->offsets[0] + 16*16;
	r_notexture_mip->offsets[2] = r_notexture_mip->offsets[1] + 8*8;
	r_notexture_mip->offsets[3] = r_notexture_mip->offsets[2] + 4*4;
	
	for (m=0 ; m<4 ; m++)
	{
		dest = (byte *)r_notexture_mip + r_notexture_mip->offsets[m];
		for (y=0 ; y< (16>>m) ; y++)
			for (x=0 ; x< (16>>m) ; x++)
			{
				if (  (y< (8>>m) ) ^ (x< (8>>m) ) )
					*dest++ = 0;
				else
					*dest++ = 0xff;
			}
	}	
}

byte	dottexture[8][8] =
{
	{0,1,1,0,0,0,0,0},
	{1,1,1,1,0,0,0,0},
	{1,1,1,1,0,0,0,0},
	{0,1,1,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
};
void R_InitParticleTexture (void)
{
	int		x,y;
	byte	data[8][8][4];

	//
	// particle texture
	//
	particletexture = texture_extension_number++;
    GL_Bind(particletexture);

	for (x=0 ; x<8 ; x++)
	{
		for (y=0 ; y<8 ; y++)
		{
			data[y][x][0] = 255;
			data[y][x][1] = 255;
			data[y][x][2] = 255;
			data[y][x][3] = dottexture[x][y]*255;
		}
	}
	glTexImage2D (GL_TEXTURE_2D, 0, gl_alpha_format, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/*
===============
R_Envmap_f

Grab six views for environment mapping tests
===============
*/
void R_Envmap_f (void)
{
	byte	buffer[256*256*4];
	char	name[1024];

	glDrawBuffer  (GL_FRONT);
	glReadBuffer  (GL_FRONT);
	envmap = true;

	r_refdef.vrect.x = 0;
	r_refdef.vrect.y = 0;
	r_refdef.vrect.width = 256;
	r_refdef.vrect.height = 256;

	r_refdef.viewangles[0] = 0;
	r_refdef.viewangles[1] = 0;
	r_refdef.viewangles[2] = 0;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	COM_WriteFile ("env0.rgb", buffer, sizeof(buffer));		

	r_refdef.viewangles[1] = 90;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	COM_WriteFile ("env1.rgb", buffer, sizeof(buffer));		

	r_refdef.viewangles[1] = 180;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	COM_WriteFile ("env2.rgb", buffer, sizeof(buffer));		

	r_refdef.viewangles[1] = 270;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	COM_WriteFile ("env3.rgb", buffer, sizeof(buffer));		

	r_refdef.viewangles[0] = -90;
	r_refdef.viewangles[1] = 0;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	COM_WriteFile ("env4.rgb", buffer, sizeof(buffer));		

	r_refdef.viewangles[0] = 90;
	r_refdef.viewangles[1] = 0;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	COM_WriteFile ("env5.rgb", buffer, sizeof(buffer));		

	envmap = false;
	glDrawBuffer  (GL_BACK);
	glReadBuffer  (GL_BACK);
	GL_EndRendering ();
}

/*
===============
R_Init
===============
*/
void R_Init (void)
{	
	extern byte *hunk_base;
	extern cvar_t gl_finish;

	Cmd_AddCommand ("timerefresh", R_TimeRefresh_f);	
	Cmd_AddCommand ("envmap", R_Envmap_f);	
	Cmd_AddCommand ("pointfile", R_ReadPointFile_f);	

    // add the command to switch renderers. -- Alex

    Cmd_AddCommand( "r_load", R_dr_LoadRenderer_f );

    // load default renderer
    if( !R_dr_LoadRenderer_helper( "dr_default.so" ) ) {
        Sys_Error( "could not load default renderer!\n" );
    }

	Cvar_RegisterVariable (&r_norefresh);
	Cvar_RegisterVariable (&r_lightmap);
	Cvar_RegisterVariable (&r_fullbright);
	Cvar_RegisterVariable (&r_drawentities);
	Cvar_RegisterVariable (&r_drawviewmodel);
	Cvar_RegisterVariable (&r_shadows);
	Cvar_RegisterVariable (&r_mirroralpha);
	Cvar_RegisterVariable (&r_wateralpha);
	Cvar_RegisterVariable (&r_dynamic);
	Cvar_RegisterVariable (&r_novis);
	Cvar_RegisterVariable (&r_speeds);

	Cvar_RegisterVariable (&gl_finish);
	Cvar_RegisterVariable (&gl_clear);
	Cvar_RegisterVariable (&gl_texsort);

 	if (gl_mtexable)
		Cvar_SetValue ("gl_texsort", 0.0);

	Cvar_RegisterVariable (&gl_cull);
	Cvar_RegisterVariable (&gl_smoothmodels);
	Cvar_RegisterVariable (&gl_affinemodels);
	Cvar_RegisterVariable (&gl_polyblend);
	Cvar_RegisterVariable (&gl_flashblend);
	Cvar_RegisterVariable (&gl_playermip);
	Cvar_RegisterVariable (&gl_nocolors);

	Cvar_RegisterVariable (&gl_keeptjunctions);
	Cvar_RegisterVariable (&gl_reporttjunctions);

	Cvar_RegisterVariable (&gl_doubleeyes);

	R_InitParticles ();
	R_InitParticleTexture ();

#ifdef GLTEST
	Test_Init ();
#endif

	playertextures = texture_extension_number;
	texture_extension_number += 16;
}

/*
===============
R_TranslatePlayerSkin

Translates a skin texture by the per-player color lookup
===============
*/
void R_TranslatePlayerSkin (int playernum)
{
	int		top, bottom;
	byte	translate[256];
	unsigned	translate32[256];
	int		i, j, s;
	model_t	*model;
	aliashdr_t *paliashdr;
	byte	*original;
	unsigned	pixels[512*256], *out;
	unsigned	scaled_width, scaled_height;
	int			inwidth, inheight;
	byte		*inrow;
	unsigned	frac, fracstep;
	extern	byte		**player_8bit_texels_tbl;

	GL_DisableMultitexture();

	top = cl.scores[playernum].colors & 0xf0;
	bottom = (cl.scores[playernum].colors &15)<<4;

	for (i=0 ; i<256 ; i++)
		translate[i] = i;

	for (i=0 ; i<16 ; i++)
	{
		if (top < 128)	// the artists made some backwards ranges.  sigh.
			translate[TOP_RANGE+i] = top+i;
		else
			translate[TOP_RANGE+i] = top+15-i;
				
		if (bottom < 128)
			translate[BOTTOM_RANGE+i] = bottom+i;
		else
			translate[BOTTOM_RANGE+i] = bottom+15-i;
	}

	//
	// locate the original skin pixels
	//
	currententity = &cl_entities[1+playernum];
	model = currententity->model;
	if (!model)
		return;		// player doesn't have a model yet
	if (model->type != mod_alias)
		return; // only translate skins on alias models

	paliashdr = (aliashdr_t *)Mod_Extradata (model);
	s = paliashdr->skinwidth * paliashdr->skinheight;
	if (currententity->skinnum < 0 || currententity->skinnum >= paliashdr->numskins) {
		Con_Printf("(%d): Invalid player skin #%d\n", playernum, currententity->skinnum);
		original = (byte *)paliashdr + paliashdr->texels[0];
	} else
		original = (byte *)paliashdr + paliashdr->texels[currententity->skinnum];
	if (s & 3)
		Sys_Error ("R_TranslateSkin: s&3");

	inwidth = paliashdr->skinwidth;
	inheight = paliashdr->skinheight;

	// because this happens during gameplay, do it fast
	// instead of sending it through gl_upload 8
    GL_Bind(playertextures + playernum);

#if 0
	byte	translated[320*200];

	for (i=0 ; i<s ; i+=4)
	{
		translated[i] = translate[original[i]];
		translated[i+1] = translate[original[i+1]];
		translated[i+2] = translate[original[i+2]];
		translated[i+3] = translate[original[i+3]];
	}


	// don't mipmap these, because it takes too long
	GL_Upload8 (translated, paliashdr->skinwidth, paliashdr->skinheight, false, false, true);
#else
	scaled_width = gl_max_size.value < 512 ? gl_max_size.value : 512;
	scaled_height = gl_max_size.value < 256 ? gl_max_size.value : 256;

	// allow users to crunch sizes down even more if they want
	scaled_width >>= (int)gl_playermip.value;
	scaled_height >>= (int)gl_playermip.value;

	if (VID_Is8bit()) { // 8bit texture upload
		byte *out2;

		out2 = (byte *)pixels;
		memset(pixels, 0, sizeof(pixels));
		fracstep = inwidth*0x10000/scaled_width;
		for (i=0 ; i<scaled_height ; i++, out2 += scaled_width)
		{
			inrow = original + inwidth*(i*inheight/scaled_height);
			frac = fracstep >> 1;
			for (j=0 ; j<scaled_width ; j+=4)
			{
				out2[j] = translate[inrow[frac>>16]];
				frac += fracstep;
				out2[j+1] = translate[inrow[frac>>16]];
				frac += fracstep;
				out2[j+2] = translate[inrow[frac>>16]];
				frac += fracstep;
				out2[j+3] = translate[inrow[frac>>16]];
				frac += fracstep;
			}
		}

		GL_Upload8_EXT ((byte *)pixels, scaled_width, scaled_height, false, false);
		return;
	}

	for (i=0 ; i<256 ; i++)
		translate32[i] = d_8to24table[translate[i]];

	out = pixels;
	fracstep = inwidth*0x10000/scaled_width;
	for (i=0 ; i<scaled_height ; i++, out += scaled_width)
	{
		inrow = original + inwidth*(i*inheight/scaled_height);
		frac = fracstep >> 1;
		for (j=0 ; j<scaled_width ; j+=4)
		{
			out[j] = translate32[inrow[frac>>16]];
			frac += fracstep;
			out[j+1] = translate32[inrow[frac>>16]];
			frac += fracstep;
			out[j+2] = translate32[inrow[frac>>16]];
			frac += fracstep;
			out[j+3] = translate32[inrow[frac>>16]];
			frac += fracstep;
		}
	}
	glTexImage2D (GL_TEXTURE_2D, 0, gl_solid_format, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif

}


/*
===============
R_NewMap
===============
*/
void R_NewMap (void)
{
	int		i;
	
	for (i=0 ; i<256 ; i++)
		d_lightstylevalue[i] = 264;		// normal light value

	memset (&r_worldentity, 0, sizeof(r_worldentity));
	r_worldentity.model = cl.worldmodel;

// clear out efrags in case the level hasn't been reloaded
// FIXME: is this one short?
	for (i=0 ; i<cl.worldmodel->numleafs ; i++)
		cl.worldmodel->leafs[i].efrags = NULL;
		 	
	r_viewleaf = NULL;
	R_ClearParticles ();

	GL_BuildLightmaps ();

	// identify sky texture
	skytexturenum = -1;
	mirrortexturenum = -1;
	for (i=0 ; i<cl.worldmodel->numtextures ; i++)
	{
		if (!cl.worldmodel->textures[i])
			continue;
		if (!Q_strncmp(cl.worldmodel->textures[i]->name,"sky",3) )
			skytexturenum = i;
		if (!Q_strncmp(cl.worldmodel->textures[i]->name,"window02_1",10) )
			mirrortexturenum = i;
 		cl.worldmodel->textures[i]->texturechain = NULL;
	}
#ifdef QUAKE2
	R_LoadSkys ();
#endif
}


/*
====================
R_TimeRefresh_f

For program optimization
====================
*/
void R_TimeRefresh_f (void)
{
	int			i;
	float		start, stop, time;
	int			startangle;
	vrect_t		vr;

	glDrawBuffer  (GL_FRONT);
	glFinish ();

	start = Sys_FloatTime ();
	for (i=0 ; i<128 ; i++)
	{
		r_refdef.viewangles[1] = i/128.0*360.0;
		R_RenderView ();
	}

	glFinish ();
	stop = Sys_FloatTime ();
	time = stop-start;
	Con_Printf ("%f seconds (%f fps)\n", time, 128/time);

	glDrawBuffer  (GL_BACK);
	GL_EndRendering ();
}

void D_FlushCaches (void)
{
}




#ifdef _WIN32                        //
#define DLSYM (void *)GetProcAddress //
#define DLCLOSE FreeLibrary          //
#else                                //JG
#define DLSYM dlsym                  //
#define DLCLOSE dlclose              //
#endif                               //

int R_dr_LoadRenderer_helper( const char * lib ) {


    // this should be as big as the number of functions we pull out.
    void * tf[25];

    memset( tf, 0, sizeof( void * ) * 25 );

#ifdef _WIN32 //JG
    hnewlib = LoadLibrary( lib );
#else
    hnewlib = dlopen( lib, RTLD_LAZY | RTLD_GLOBAL );
#endif

    if( hnewlib == NULL ) {
        Con_Printf( "error: couldn't load %s\n", lib );
        return( 0 );
    }


    // pick out all the rendering functions..


    tf[0] = DLSYM( hnewlib, "dr_Set_ConPrintf" );
    if( !tf[0] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    
    
    tf[1] = DLSYM( hnewlib, "dr_Set_VectorMA" );
    if( !tf[1] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[2] = DLSYM( hnewlib, "dr_Set_GL_Bind" );
    if( !tf[2] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }


    tf[3] = DLSYM( hnewlib, "dr_Set_GL_DisableMultitexture" );
    if( !tf[3] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[4] = DLSYM( hnewlib, "dr_Set_GL_EnableMultitexture" );
    if( !tf[4] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[5] = DLSYM( hnewlib, "dr_Set_AngleVectors" );
    if( !tf[5] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[6] = DLSYM( hnewlib, "dr_Set_R_TextureAnimation" );
    if( !tf[6] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }

    
    tf[7] = DLSYM( hnewlib, "dr_Set_VectorScale" );
    if( !tf[7] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    
    


    
    
    tf[8] = DLSYM( hnewlib, "GL_SelectTexture" );
    if( !tf[8] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[9] = DLSYM( hnewlib, "R_DrawSpriteModel" );
    if( !tf[9] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[10] = DLSYM( hnewlib, "GL_DrawAliasFrame" );
    if( !tf[10] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[11] = DLSYM( hnewlib, "GL_DrawAliasShadow" );
    if( !tf[11] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[12] = DLSYM( hnewlib, "R_PolyBlend" );
    if( !tf[12] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[13] = DLSYM( hnewlib, "R_DrawSequentialPoly" );
    if( !tf[13] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }

    
    tf[14] = DLSYM( hnewlib, "DrawGLWaterPoly" );
    if( !tf[14] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[15] = DLSYM( hnewlib, "DrawGLWaterPolyLightmap" );
    if( !tf[15] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[16] = DLSYM( hnewlib, "DrawGLPoly" );
    if( !tf[16] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[17] = DLSYM( hnewlib, "R_BlendLightmaps" );
    if( !tf[17] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[18] = DLSYM( hnewlib, "R_DrawParticles" );
    if( !tf[18] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[19] = DLSYM( hnewlib, "EmitWaterPolys" );
    if( !tf[19] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }
    

    tf[20] = DLSYM( hnewlib, "EmitSkyPolys" );
    if( !tf[20] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }



    tf[21] = DLSYM( hnewlib, "dr_Init" );
    if( !tf[21] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }



    tf[22] = DLSYM( hnewlib, "dr_Shutdown" );
    if( !tf[22] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }


    tf[23] = DLSYM( hnewlib, "dr_Set_Cvar_RegisterVariable" );
    if( !tf[23] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }


    tf[24] = DLSYM( hnewlib, "dr_Set_Cvar_UnRegisterVariable" );
    if( !tf[24] ) {
        Con_Printf( "error: invalid rendering dll\n" );
        DLCLOSE( hnewlib );
        return( 0 );
    }




    // get rid of the old library.
    if( holdlib != NULL ) {
        dr_Shutdown();
        DLCLOSE( holdlib );
    }

    holdlib = hnewlib;


    // load up the new one properly.
    
    dr_Set_ConPrintf = tf[0];
    dr_Set_VectorMA = tf[1];
    dr_Set_GL_Bind = tf[2];
    dr_Set_GL_DisableMultitexture = tf[3];
    dr_Set_GL_EnableMultitexture = tf[4];
    dr_Set_AngleVectors = tf[5];
    dr_Set_R_TextureAnimation = tf[6];
    dr_Set_VectorScale = tf[7];
    GL_SelectTexture  = tf[8];
    R_DrawSpriteModel  = tf[9];
    GL_DrawAliasFrame  = tf[10];
    GL_DrawAliasShadow  = tf[11];
    R_PolyBlend  = tf[12];
    R_DrawSequentialPoly  = tf[13];
    DrawGLWaterPoly  = tf[14];
    DrawGLWaterPolyLightmap  = tf[15];
    DrawGLPoly  = tf[16];
    R_BlendLightmaps  = tf[17];
    R_DrawParticles  = tf[18];
    EmitWaterPolys  = tf[19];
    EmitSkyPolys  = tf[20];
    dr_Init = tf[21];
    dr_Shutdown = tf[22];
    dr_Set_Cvar_RegisterVariable = tf[23];
    dr_Set_Cvar_UnRegisterVariable = tf[24];


    // call all the set functions.

    dr_Set_ConPrintf( Con_Printf );
    dr_Set_VectorMA( VectorMA );
    dr_Set_GL_Bind( GL_Bind );
    dr_Set_GL_DisableMultitexture( GL_DisableMultitexture );
    dr_Set_GL_EnableMultitexture( GL_EnableMultitexture );
    dr_Set_AngleVectors( AngleVectors );
    dr_Set_R_TextureAnimation( R_TextureAnimation );
    dr_Set_VectorScale( VectorScale );
    dr_Set_Cvar_RegisterVariable( Cvar_RegisterVariable );
    dr_Set_Cvar_UnRegisterVariable( Cvar_UnRegisterVariable );

    // call the init function.

    dr_Init();

    return( 1 );

    
}





//////////////////
// R_dr_LoadRenderer_f -- load a new dynamic renderer.
//

void R_dr_LoadRenderer_f( void ) {

    if( Cmd_Argc() < 2 ) {
        Con_Printf( "usage: r_load <dynamic renderer>\n" );
        return;
    }

    if( !R_dr_LoadRenderer_helper( Cmd_Argv( 1 ) ) ) {
        Con_Printf( "could not load renderer!\n" );
    }
    
}









