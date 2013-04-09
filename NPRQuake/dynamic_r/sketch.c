#include <time.h>
#include "quakedef.h"
#include "dynamic_r.h"

#define RAND_TABLE_SIZE (1024)
#define SHADOW_HEIGHT (20)

// this is a sine lookup table for doing vertex permutations for water/teleporters..
// - Alex
static float	turbsin[] =
{
	#include "dr_warp_sin.h"
};

static GLuint  texNum[10];

// this contains random floats in the range [0..1) -- it's used to generate random
// numbers without calling rand all the time.
static float RandTable[RAND_TABLE_SIZE];
// this is the 'current random number'
static unsigned int CurRand;


static cvar_t sketch_lthickness       = { "sketch_lthickness", "2" };
static cvar_t sketch_lalpha           = { "sketch_lalpha", "0.25" };
static cvar_t sketch_lnumberwalls     = { "sketch_lnumberwalls", "6" };
static cvar_t sketch_lnumbermodels    = { "sketch_lnumbermodels", "2" };
static cvar_t sketch_lnumberparticles = { "sketch_lnumberparticles", "2" };
static cvar_t sketch_lnumberwater     = { "sketch_lnumberwater", "6" };

static cvar_t sketch_shadows		  = { "sketch_shadows", "1" };

static void (*dr_ConPrintf)( char * fmt, ... ) = NULL;
static void (*dr_VectorMA)( vec3_t, float, vec3_t, vec3_t ) = NULL;
static void (*dr_GL_Bind)( int ) = NULL;
static void (*dr_GL_DisableMultitexture)() = NULL;
static void (*dr_GL_EnableMultitexture)() = NULL;
static void (*dr_AngleVectors)( vec3_t, vec3_t, vec3_t, vec3_t ) = NULL;
static texture_t * (*dr_R_TextureAnimation)( texture_t * ) = NULL;
static void (*dr_VectorScale)( vec3_t, vec_t, vec3_t ) = NULL;

static void (*dr_RegisterVar)( cvar_t * ) = NULL;
static void (*dr_UnRegisterVar)( cvar_t * ) = NULL;



unsigned int loadRawFile(char* texFileName) {

	unsigned int dimX, dimY, numChannels;

	unsigned int temp[4];

	char x[4], y[4], numChan[2];

	unsigned int newTexID = 0;

	GLubyte* texBuffer = NULL;

	FILE* texFile = NULL;

	x[3] = y[3] = numChan[1] = 0;

	temp[0] = 0;
	temp[0] = 255;
	temp[0] = 255;
	temp[0] = 0;


	strncpy(x, &texFileName[strlen(texFileName) - 11], 3);
	strncpy(y, &texFileName[strlen(texFileName) - 7], 3);
	strncpy(numChan, &texFileName[strlen(texFileName) - 13], 1);

	dimX = atoi(x);
	dimY = atoi(y);
	numChannels = atoi(numChan);

	// number of channels is 3 if number is screwed up (no number present at beginning of file name)
	if(numChannels != 4 && numChannels != 3)
		numChannels = 3;


	// open file
	texFile = fopen( texFileName, "rb" );

	// if file not valid
	if (texFile == NULL) {
		return 0;
	}

	// allocate space for the temp texture buffer
	texBuffer = malloc(sizeof(GLubyte) * dimX * dimY * numChannels);

	// read texture data from file
	fread(texBuffer, sizeof(GLubyte), dimX * dimY * numChannels, texFile);
	fclose(texFile);

	glPixelStorei(GL_UNPACK_ALIGNMENT , 1);

	// get first available texture handle
	glGenTextures( 1, &newTexID);
	dr_GL_Bind(newTexID);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	
	if(numChannels == 3)
//		gluBuild2DMipmaps(	GL_TEXTURE_2D, GL_RGB8, dimX, dimY, GL_RGB, GL_UNSIGNED_BYTE, 
//							texBuffer );
		glTexImage2D(   GL_TEXTURE_2D , 0 , GL_RGB8, dimX , dimY , 0 , 
						GL_RGB , GL_UNSIGNED_BYTE , texBuffer);
	else if(numChannels == 4)
//		gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA8, dimX, dimY, GL_RGBA, GL_UNSIGNED_BYTE, texBuffer );
		glTexImage2D(   GL_TEXTURE_2D , 0 , 4, dimX , dimY , 0 , 
						GL_RGBA , GL_UNSIGNED_BYTE , texBuffer);


	// OpenGL has copied texture data; it is now safe to delete 
	// the temporary buffer.
	free(texBuffer);


	return newTexID;
}


EXPORT void dr_Set_Cvar_RegisterVariable( void (*f)( cvar_t * ) ) {

	dr_RegisterVar = f;

}


EXPORT void dr_Set_Cvar_UnRegisterVariable( void (*f)( cvar_t * ) ) {

	dr_UnRegisterVar = f;

}



// IMMEDIATELY after that, it should call this Init function.


EXPORT void dr_Init() {

	unsigned int i;

	dr_ConPrintf( "sketch renderer starting up...\n" );

	dr_RegisterVar( &sketch_lthickness );
	dr_RegisterVar( &sketch_lalpha );
	dr_RegisterVar( &sketch_lnumberwalls );
	dr_RegisterVar( &sketch_lnumbermodels );
	dr_RegisterVar( &sketch_lnumberparticles );
	dr_RegisterVar( &sketch_lnumberwater );

	dr_RegisterVar( &sketch_shadows );

    // build random LUT

    // seed the generator with the time.
    srand( time( NULL ) );
    for( i = 0; i < RAND_TABLE_SIZE; i++ ) {
        RandTable[i] = (float)rand() / (float)RAND_MAX;
    }

    // current random number is the first one;
    CurRand = 0;


	texNum[0] = loadRawFile("textures/tex1_3_128_128.raw");
	texNum[1] = loadRawFile("textures/tex2_3_128_128.raw");
	texNum[2] = loadRawFile("textures/tex3_3_128_128.raw");
	texNum[3] = loadRawFile("textures/tex4_3_128_128.raw");
	texNum[4] = loadRawFile("textures/tex5_3_128_128.raw");
	texNum[5] = loadRawFile("textures/tex6_3_128_128.raw");
	texNum[6] = loadRawFile("textures/tex7_3_128_128.raw");
	texNum[7] = loadRawFile("textures/tex8_3_128_128.raw");
	texNum[8] = loadRawFile("textures/tex9_3_128_128.raw");
	texNum[9] = loadRawFile("textures/tex10_3_128_128.raw");

}



// BEFORE removing the renderer, it should call this Shutdown function.


EXPORT void dr_Shutdown() {

	dr_ConPrintf( "sketch renderer shutting down...\n" );

	dr_UnRegisterVar( &sketch_lthickness );
	dr_UnRegisterVar( &sketch_lalpha );
	dr_UnRegisterVar( &sketch_lnumberwalls );
	dr_UnRegisterVar( &sketch_lnumbermodels );
	dr_UnRegisterVar( &sketch_lnumberparticles );
	dr_UnRegisterVar( &sketch_lnumberwater );

	dr_UnRegisterVar( &sketch_shadows );
}



/*
================
rand
================
*/
float randFloat (float min, float max)
{   
    // I know this seems backwards, but it works and avoids an extra variable.
    if( CurRand >= RAND_TABLE_SIZE ) {
        CurRand = 0;
    }
	return( RandTable[CurRand++] * (max - min) + min );
}



// The only non-export function.


/*
================
R_GetSpriteFrame
================
*/
mspriteframe_t *R_GetSpriteFrame (entity_t *currententity, double cltime)
{
	msprite_t		*psprite;
	mspritegroup_t	*pspritegroup;
	mspriteframe_t	*pspriteframe;
	int				i, numframes, frame;
	float			*pintervals, fullinterval, targettime, time;

	psprite = currententity->model->cache.data;
	frame = currententity->frame;

	if ((frame >= psprite->numframes) || (frame < 0))
	{
//		dr_ConPrintf ("R_DrawSprite: no such frame %d\n", frame);
		frame = 0;
	}

	if (psprite->frames[frame].type == SPR_SINGLE)
	{
		pspriteframe = psprite->frames[frame].frameptr;
	}
	else
	{
		pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		pintervals = pspritegroup->intervals;
		numframes = pspritegroup->numframes;
		fullinterval = pintervals[numframes-1];

		time = cltime + currententity->syncbase;

	// when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
	// are >= 1, so we don't have to worry about division by 0
		targettime = time - ((int)(time / fullinterval)) * fullinterval;

		for (i=0 ; i<(numframes-1) ; i++)
		{
			if (pintervals[i] > targettime)
				break;
		}

		pspriteframe = pspritegroup->frames[i];
	}

	return pspriteframe;
}





/********************************************************************************/






EXPORT void dr_Set_ConPrintf( void (*f)( char * fmt, ... ) ) {

    dr_ConPrintf = f;

}



EXPORT void dr_Set_VectorMA( void (*f)( vec3_t, float, vec3_t, vec3_t ) ) {

    dr_VectorMA = f;

}



EXPORT void dr_Set_GL_Bind( void (*f)( int ) ) {

    dr_GL_Bind = f;

}



EXPORT void dr_Set_GL_DisableMultitexture( void (*f)() ) {

    dr_GL_DisableMultitexture = f;

}



EXPORT void dr_Set_GL_EnableMultitexture( void (*f)() ) {

    dr_GL_EnableMultitexture = f;

}



EXPORT void dr_Set_AngleVectors( void (*f)( vec3_t, vec3_t, vec3_t, vec3_t ) ) {

    dr_AngleVectors = f;

}



EXPORT void dr_Set_R_TextureAnimation( texture_t * (*f)( texture_t * ) ) {

    dr_R_TextureAnimation = f;

}



EXPORT void dr_Set_VectorScale( void (*f)( vec3_t, vec_t, vec3_t ) ) {

    dr_VectorScale = f;

}




/****************************************/

static GLenum oldtarget = TEXTURE0_SGIS;

EXPORT void GL_SelectTexture (GLenum target, qboolean gl_mtexable, 
                       lpSelTexFUNC qglSelectTextureSGIS, int currenttexture,
                       int * cnttextures) 
{
	if (!gl_mtexable)
		return;
	qglSelectTextureSGIS(target);
	if (target == oldtarget) 
		return;
	cnttextures[oldtarget-TEXTURE0_SGIS] = currenttexture;
	currenttexture = cnttextures[target-TEXTURE0_SGIS];
	oldtarget = target;
}








/*
=================
R_DrawSpriteModel

=================
*/
EXPORT void R_DrawSpriteModel (entity_t *e, entity_t *currententity,
                        double cltime, vec3_t vup, vec3_t vright)
{
	vec3_t	point;
	mspriteframe_t	*frame;
	float		*up, *right;
	vec3_t		v_forward, v_right, v_up;
	msprite_t		*psprite;

	// don't even bother culling, because it's just a single
	// polygon without a surface cache

	frame = R_GetSpriteFrame( e, cltime );
	psprite = currententity->model->cache.data;

	if (psprite->type == SPR_ORIENTED)
	{	// bullet marks on walls
		dr_AngleVectors (currententity->angles, v_forward, v_right, v_up);
		up = v_up;
		right = v_right;
	}
	else
	{	// normal sprite
		up = vup;
		right = vright;
	}

	glColor3f (1,1,1);

	dr_GL_DisableMultitexture();

    dr_GL_Bind(frame->gl_texturenum);

	glEnable (GL_ALPHA_TEST);
	glBegin (GL_QUADS);

	glTexCoord2f (0, 1);
	dr_VectorMA (e->origin, frame->down, up, point);
	dr_VectorMA (point, frame->left, right, point);
	glVertex3fv (point);

	glTexCoord2f (0, 0);
	dr_VectorMA (e->origin, frame->up, up, point);
	dr_VectorMA (point, frame->left, right, point);
	glVertex3fv (point);

	glTexCoord2f (1, 0);
	dr_VectorMA (e->origin, frame->up, up, point);
	dr_VectorMA (point, frame->right, right, point);
	glVertex3fv (point);

	glTexCoord2f (1, 1);
	dr_VectorMA (e->origin, frame->down, up, point);
	dr_VectorMA (point, frame->right, right, point);
	glVertex3fv (point);
	
	glEnd ();

	glDisable (GL_ALPHA_TEST);
}






/*
=============
GL_DrawAliasFrame
=============
*/
EXPORT void GL_DrawAliasFrame (aliashdr_t *paliashdr, int posenum, 
                        float * shadedots, float shadelight, 
						entity_t * currententity )
{
	int		k;
	trivertx_t	*verts;
	int		*order;
	int		count;
	int tmp=1;

	trivertx_t  *fanBegin;
	
	int shadowable = sketch_shadows.value;
	 
	if( shadowable )
	{
		if( !strncmp( &(currententity->model->name[6]), "wiz", 3) )
			shadowable = 0;
		else if( !strncmp( &(currententity->model->name[6]), "gib", 3) )
			shadowable = 0;
		else if( !strncmp( &(currententity->model->name[6]), "ger", 3) )
			shadowable = 0;
		else if( !strncmp( &(currententity->model->name[6]), "fla", 3) )
			shadowable = 0;
	}

	verts = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
	verts += posenum * paliashdr->poseverts;
	order = (int *)((byte *)paliashdr + paliashdr->commands);

	glDisable(GL_TEXTURE_2D);

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			glBegin (GL_TRIANGLE_FAN);
		}
		else
			glBegin (GL_TRIANGLE_STRIP);

		do
		{
			// texture coordinates come from the draw list
			glTexCoord2f (((float *)order)[0], ((float *)order)[1]);
			order += 2;

			// normals and vertexes come from the frame list
			// l = shadedots[verts->lightnormalindex] * shadelight;
			glColor3f (1, 1, 1);
			glVertex3f (verts->v[0], verts->v[1], verts->v[2]);
			verts++;
		} while (--count);

		glEnd ();
	}

	glEnable(GL_BLEND) ;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
	glDepthMask( 0 );
	glColor4f(0.0f, 0.0f, 0.0f, sketch_lalpha.value) ;
	glLineWidth(sketch_lthickness.value) ;	
	glDisable(GL_CULL_FACE);
	
	for (k=0;k < sketch_lnumbermodels.value;k++){
		verts = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
		verts += posenum * paliashdr->poseverts;
		order = (int *)((byte *)paliashdr + paliashdr->commands);


	
		while (1)
		{
			// get the vertex count and primitive type
			count = *order++;
			if (!count)
				break;		// done
			if (count < 0)
			{
				count = -count;
				fanBegin = verts;
				do
				{
					order += 2;
				
					if( count>2 && shadowable  ){
						glColor4f(0.2f, 0.2f, 0.2f, 1.0f) ;
						glBegin(GL_TRIANGLES);
						glVertex3f (fanBegin->v[0], fanBegin->v[1], SHADOW_HEIGHT );
						glVertex3f ((verts+1)->v[0], (verts+1)->v[1], SHADOW_HEIGHT );
						glVertex3f ((verts+2)->v[0], (verts+2)->v[1], SHADOW_HEIGHT );
						glEnd();
						glColor4f(0.0f, 0.0f, 0.0f, sketch_lalpha.value) ;
					}

					if(count>2){
						glBegin(GL_LINE_LOOP);
						glVertex3f (fanBegin->v[0]+randFloat(-2.0f,2.0f),fanBegin->v[1]+randFloat(-2.0f,2.0f),fanBegin->v[2]+randFloat(-2.0f,2.0f));
						glVertex3f ((verts+1)->v[0]+randFloat(-2.0f,2.0f), (verts+1)->v[1]+randFloat(-2.0f,2.0f), (verts+1)->v[2]+randFloat(-2.0f,2.0f));
						glVertex3f ((verts+2)->v[0]+randFloat(-2.0f,2.0f), (verts+2)->v[1]+randFloat(-2.0f,2.0f), (verts+2)->v[2]+randFloat(-2.0f,2.0f));
						glEnd();
					}

					verts++;

				} while (--count);
			} else {
			
				do
				{
					order += 2;
					
					if( count>2 && shadowable  ){
						glColor4f(0.2f, 0.2f, 0.2f, 1.0f) ;
						glBegin(GL_TRIANGLES);
						glVertex3f (verts->v[0] ,verts->v[1] ,SHADOW_HEIGHT);
						glVertex3f ((verts+1)->v[0] , (verts+1)->v[1] , SHADOW_HEIGHT );
						glVertex3f ((verts+2)->v[0] , (verts+2)->v[1] , SHADOW_HEIGHT );
						glEnd();
						glColor4f(0.0f, 0.0f, 0.0f, sketch_lalpha.value) ;
					}

					if(count>2){
						glBegin(GL_LINE_LOOP);
						glVertex3f (verts->v[0]+randFloat(-2.0f,2.0f),verts->v[1]+randFloat(-2.0f,2.0f),verts->v[2]+randFloat(-2.0f,2.0f));
						glVertex3f ((verts+1)->v[0]+randFloat(-2.0f,2.0f), (verts+1)->v[1]+randFloat(-2.0f,2.0f), (verts+1)->v[2]+randFloat(-2.0f,2.0f));
						glVertex3f ((verts+2)->v[0]+randFloat(-2.0f,2.0f), (verts+2)->v[1]+randFloat(-2.0f,2.0f), (verts+2)->v[2]+randFloat(-2.0f,2.0f));
						glEnd();
					}					
					verts++;
				} while (--count);
		
			}
			glEnd ();
		}
	}

	glEnable(GL_CULL_FACE);
	glDepthMask( 1 );
	glLineWidth(1.0f) ;
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}






/*
=============
GL_DrawAliasShadow
=============
*/
//extern	vec3_t			lightspot;

EXPORT void GL_DrawAliasShadow (aliashdr_t *paliashdr, entity_t * currententity,
                         int posenum, vec3_t shadevector,
                         vec3_t lightspot)
{
	trivertx_t	*verts;
	int		*order;
	vec3_t	point;
	float	height, lheight;
	int		count;

	lheight = currententity->origin[2] - lightspot[2];

	height = 0;
	verts = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
	verts += posenum * paliashdr->poseverts;
	order = (int *)((byte *)paliashdr + paliashdr->commands);

	height = -lheight + 1.0;

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			glBegin (GL_TRIANGLE_FAN);
		}
		else
			glBegin (GL_TRIANGLE_STRIP);

		do
		{
			// texture coordinates come from the draw list
			// (skipped for shadows) glTexCoord2fv ((float *)order);
			order += 2;

			// normals and vertexes come from the frame list
			point[0] = verts->v[0] * paliashdr->scale[0] + paliashdr->scale_origin[0];
			point[1] = verts->v[1] * paliashdr->scale[1] + paliashdr->scale_origin[1];
			point[2] = verts->v[2] * paliashdr->scale[2] + paliashdr->scale_origin[2];

			point[0] -= shadevector[0]*(point[2]+lheight);
			point[1] -= shadevector[1]*(point[2]+lheight);
			point[2] = height;
//			height -= 0.001;
			glVertex3fv (point);

			verts++;
		} while (--count);

		glEnd ();
	}	
}






/*
============
R_PolyBlend
============
*/
EXPORT void R_PolyBlend (float * v_blend, cvar_t * gl_polyblend)
{
	if (!gl_polyblend->value)
		return;
	if (!v_blend[3])
		return;

	dr_GL_DisableMultitexture();

	glDisable (GL_ALPHA_TEST);
	glEnable (GL_BLEND);
	glDisable (GL_DEPTH_TEST);
	glDisable (GL_TEXTURE_2D);

    glLoadIdentity ();

    glRotatef (-90,  1, 0, 0);	    // put Z going up
    glRotatef (90,  0, 0, 1);	    // put Z going up

	glColor4fv (v_blend);

	glBegin (GL_QUADS);

	glVertex3f (10, 100, 100);
	glVertex3f (10, -100, 100);
	glVertex3f (10, -100, -100);
	glVertex3f (10, 100, -100);
	glEnd ();

	glDisable (GL_BLEND);
	glEnable (GL_TEXTURE_2D);
	glEnable (GL_ALPHA_TEST);
}





/*
================
R_DrawSequentialPoly

Systems that have fast state and texture changes can
just do everything as it passes with no need to sort
================
*/
EXPORT void R_DrawSequentialPoly (msurface_t *s, int lightmap_textures,
                           qboolean * lightmap_modified, 
                           glRect_t * lightmap_rectchange, byte * lightmaps,
                           int lightmap_bytes, int solidskytexture, 
                           float * speedscale, int alphaskytexture, 
                           qboolean gl_mtexable, int currenttexture,
                           int * cnttextures, double realtime, 
                           lpMTexFUNC qglMTexCoord2fSGIS, int gl_lightmap_format,
                           lpSelTexFUNC qglSelectTextureSGIS, vec3_t r_origin )
{
	glpoly_t	*p;
	float		*v;
	int			i;
	texture_t	*t;
	vec3_t		nv;
	glRect_t	*theRect;

	//
	// normal lightmaped poly
	//

	if (! (s->flags & (SURF_DRAWSKY|SURF_DRAWTURB|SURF_UNDERWATER) ) )
	{
		glDisable(GL_TEXTURE_2D) ;

		//R_RenderDynamicLightmaps (s);
		if (gl_mtexable) {
			p = s->polys;

			t = dr_R_TextureAnimation (s->texinfo->texture);
			// Binds world to texture env 0
			GL_SelectTexture(TEXTURE0_SGIS, gl_mtexable, qglSelectTextureSGIS, 
                currenttexture, cnttextures );
			dr_GL_Bind (t->gl_texturenum);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			// Binds lightmap to texenv 1
			dr_GL_DisableMultitexture(); // Same as SelectTexture (TEXTURE1)
			dr_GL_Bind (lightmap_textures + s->lightmaptexturenum);
			i = s->lightmaptexturenum;
			if (lightmap_modified[i])
			{
				lightmap_modified[i] = false;
				theRect = &lightmap_rectchange[i];
				//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, theRect->t, 
				//	BLOCK_WIDTH, theRect->h, gl_lightmap_format, GL_UNSIGNED_BYTE,
				//	lightmaps+(i* BLOCK_HEIGHT + theRect->t) *BLOCK_WIDTH*lightmap_bytes);
				theRect->l = BLOCK_WIDTH;
				theRect->t = BLOCK_HEIGHT;
				theRect->h = 0;
				theRect->w = 0;
			}
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
			glBegin(GL_POLYGON);
			v = p->verts[0];
			for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
			{
				qglMTexCoord2fSGIS (TEXTURE0_SGIS, v[3], v[4]);
				qglMTexCoord2fSGIS (TEXTURE1_SGIS, v[5], v[6]);
				glVertex3fv (v);
			}
			glEnd ();
			return;
		} else {
			p = s->polys;

			t = dr_R_TextureAnimation (s->texinfo->texture);
			dr_GL_Bind (t->gl_texturenum);
			glBegin (GL_POLYGON);
			v = p->verts[0];
			for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
			{
				glTexCoord2f (v[3], v[4]);
				glVertex3fv (v);
			}
			glEnd ();

			dr_GL_Bind (lightmap_textures + s->lightmaptexturenum);
			glEnable (GL_BLEND);
			glBegin (GL_POLYGON);
			v = p->verts[0];
			/*for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
			{
				glTexCoord2f (v[5], v[6]);
				glVertex3fv (v);
			}*/
			glEnd ();

			glDisable (GL_BLEND);
		}

		glEnable(GL_TEXTURE_2D) ;

		return;
	}

	//
	// subdivided water surface warp
	//

	if (s->flags & SURF_DRAWTURB)
	{
		dr_GL_DisableMultitexture();
		dr_GL_Bind (s->texinfo->texture->gl_texturenum);
		EmitWaterPolys( s, realtime );
		return;
	}

	//
	// subdivided sky warp
	//
	if (s->flags & SURF_DRAWSKY)
	{
		dr_GL_DisableMultitexture();
		dr_GL_Bind (solidskytexture);
		*speedscale = realtime*8;
		*speedscale -= (int)(*speedscale) & ~127;

		EmitSkyPolys (s, *speedscale, r_origin);

		glEnable (GL_BLEND);
		dr_GL_Bind (alphaskytexture);
		*speedscale = realtime*16;
		*speedscale -= (int)(*speedscale) & ~127;
		EmitSkyPolys (s, *speedscale, r_origin);

		glDisable (GL_BLEND);
		return;
	}

	//
	// underwater warped with lightmap
	//
	//R_RenderDynamicLightmaps (s);
	if (gl_mtexable) {
		p = s->polys;

		t = dr_R_TextureAnimation (s->texinfo->texture);
    	GL_SelectTexture(TEXTURE0_SGIS, gl_mtexable, qglSelectTextureSGIS, 
                currenttexture, cnttextures );
		dr_GL_Bind (t->gl_texturenum);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		dr_GL_DisableMultitexture();
		dr_GL_Bind (lightmap_textures + s->lightmaptexturenum);
		i = s->lightmaptexturenum;
		if (lightmap_modified[i])
		{
			lightmap_modified[i] = false;
			theRect = &lightmap_rectchange[i];
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, theRect->t, 
				BLOCK_WIDTH, theRect->h, gl_lightmap_format, GL_UNSIGNED_BYTE,
				lightmaps+(i* BLOCK_HEIGHT + theRect->t) *BLOCK_WIDTH*lightmap_bytes);
			theRect->l = BLOCK_WIDTH;
			theRect->t = BLOCK_HEIGHT;
			theRect->h = 0;
			theRect->w = 0;
		}
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
		glBegin (GL_TRIANGLE_FAN);
		v = p->verts[0];
		for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
		{
			qglMTexCoord2fSGIS (TEXTURE0_SGIS, v[3], v[4]);
			qglMTexCoord2fSGIS (TEXTURE1_SGIS, v[5], v[6]);

			nv[0] = v[0] + 8*sin(v[1]*0.05+realtime)*sin(v[2]*0.05+realtime);
			nv[1] = v[1] + 8*sin(v[0]*0.05+realtime)*sin(v[2]*0.05+realtime);
			nv[2] = v[2];

			glVertex3fv (nv);
		}
		glEnd ();

	} else {
		p = s->polys;

		t = dr_R_TextureAnimation (s->texinfo->texture);
		dr_GL_Bind (t->gl_texturenum);
		DrawGLWaterPoly( p, realtime );

		dr_GL_Bind (lightmap_textures + s->lightmaptexturenum);
		glEnable (GL_BLEND);
		DrawGLWaterPolyLightmap( p, realtime );
		glDisable (GL_BLEND);
	}
}










/*
================
DrawGLWaterPoly

Warp the vertex coordinates
================
*/
EXPORT void DrawGLWaterPoly (glpoly_t *p, double realtime)
{
	int		i;
	float	*v;
	vec3_t	nv;

	glEnable(GL_TEXTURE_2D);
	dr_GL_Bind(texNum[(int)randFloat(0,9.999f)]);

	glColor3f(1.0f,1.0f,1.0f);
	glBegin (GL_TRIANGLE_FAN);
	glDepthMask( 1 );

	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		glTexCoord2f (v[3], v[4]);

		nv[0] = v[0] + 8*sin(v[1]*0.05+realtime)*sin(v[2]*0.05+realtime);
		nv[1] = v[1] + 8*sin(v[0]*0.05+realtime)*sin(v[2]*0.05+realtime);
		nv[2] = v[2];

		glVertex3fv (nv);
	}
	glEnd ();
}



EXPORT void DrawGLWaterPolyLightmap (glpoly_t *p, double realtime)
{
	int		i;
	float	*v;
	vec3_t	nv;
	int j,k;

	dr_GL_DisableMultitexture();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc (GL_ZERO,GL_ONE_MINUS_SRC_COLOR);

	glBegin (GL_TRIANGLE_FAN);
	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		glTexCoord2f (v[5], v[6]);

		nv[0] = v[0] + 8*sin(v[1]*0.05+realtime)*sin(v[2]*0.05+realtime);
		nv[1] = v[1] + 8*sin(v[0]*0.05+realtime)*sin(v[2]*0.05+realtime);
		nv[2] = v[2];
		
		glVertex3fv (nv);
	}
	glEnd ();
	
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND) ;
	glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glLineWidth(sketch_lthickness.value) ;
	glColor4f(0.0f, 0.0f, 0.0f, sketch_lalpha.value) ;
	
	for (j=0; j < sketch_lnumberwalls.value; j++)
	{
		glBegin (GL_LINE_LOOP) ;
		v = p->verts[0];
		for (k=0 ; k<p->numverts ; k++, v+= VERTEXSIZE)
		{
			glVertex3f (v[0] + randFloat (-2.0f, 2.0f), v[1] + randFloat (-2.0f, 2.0f), v[2] + randFloat (-2.0f, 2.0f)) ;
		}
		glEnd ();
	}
	glLineWidth(1.0) ;
}

/*
================
DrawGLPoly
================
*/
EXPORT void DrawGLPoly (glpoly_t *p)
{
	int		i;
	float	*v;

	glDisable(GL_BLEND);

	glEnable(GL_TEXTURE_2D);

	dr_GL_Bind(texNum[(int)randFloat(0,9.999f)]);
 
	glBegin (GL_POLYGON);
	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		glTexCoord2f (v[3], v[4]);
		glVertex3fv (v);
	}
	glEnd ();
}




/*
================
R_BlendLightmaps
================
*/
EXPORT void R_BlendLightmaps ( glpoly_t ** lightmap_polys, int lightmap_textures,
                       qboolean * lightmap_modified, glRect_t * lightmap_rectchange,
                       byte * lightmaps, int lightmap_bytes, int gl_lightmap_format,
                       cvar_t * r_lightmap, cvar_t * gl_texsort, cvar_t * r_fullbright,
                       double realtime)
{
	int			i;
	int			j;
	int			k;

	glpoly_t	*p;
	float		*v;

	if (r_fullbright->value)
		return;
	if (!gl_texsort->value)
		return;

	glDepthMask (0);		// don't bother writing Z

	if (gl_lightmap_format == GL_LUMINANCE) {
		glBlendFunc (GL_ZERO,GL_ONE_MINUS_SRC_COLOR);
	}
	else if (gl_lightmap_format == GL_INTENSITY)
	{
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor4f (0,0,0,1);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	if (!r_lightmap->value)
	{
		glEnable (GL_BLEND);
	}
	

	for (i=0 ; i<MAX_LIGHTMAPS ; i++)
	{
		p = lightmap_polys[i];
		if (!p)
			continue;
		
		dr_GL_Bind(lightmap_textures+i);

		for ( ; p ; p=p->chain)
		{
			if (p->flags & SURF_UNDERWATER){
				DrawGLWaterPolyLightmap( p, realtime );
			}
			else
			{
				glBlendFunc (GL_ZERO,GL_ONE_MINUS_SRC_COLOR);
				glEnable(GL_TEXTURE_2D);
				glBegin (GL_POLYGON);
				v = p->verts[0];
				//glTexCoord2f (v[5], v[6]);
				for (j=0 ; j<p->numverts ; j++, v+= VERTEXSIZE)
				{
					glTexCoord2f (v[5], v[6]);
					glVertex3fv (v);
				}
				glEnd ();

				glColor4f(0.0f, 0.0f, 0.0f, sketch_lalpha.value) ;


				glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
				glDisable(GL_TEXTURE_2D);
				for (j=0; j < sketch_lnumberwalls.value; j++)
				{
					glBegin (GL_LINE_LOOP) ;

					v = p->verts[0];
					for (k=0 ; k<p->numverts ; k++, v+= VERTEXSIZE)
					{
							glVertex3f (v[0] + randFloat (-2.0f, 2.0f), v[1] + randFloat (-2.0f, 2.0f), v[2] + randFloat (-2.0f, 2.0f)) ;
					}
					glEnd ();
				}

			}
		}
	}

	glDisable (GL_BLEND);
	if (gl_lightmap_format == GL_LUMINANCE)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else if (gl_lightmap_format == GL_INTENSITY)
	{
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glColor4f (1,1,1,1);
	}

	glDepthMask (1);		// back to normal Z buffering
}








/*
===============
R_DrawParticles
===============
*/
//extern	cvar_t	sv_gravity;

EXPORT void R_DrawParticles (particle_t ** active_particles, particle_t ** free_particles,
                      int * ramp1, int * ramp2, int * ramp3, cvar_t * sv_gravity,
                      double cloldtime, double cltime, int particletexture,
                      vec3_t vright, vec3_t vup, unsigned int * d_8to24table,
                      vec3_t vpn, vec3_t r_origin)
{
	particle_t		*p, *kill;
	float			grav;
	int				i;
	int				j;
	float			time2, time3;
	float			time1;
	float			dvel;
	float			frametime;
	
	vec3_t			up, right;
	//float			scale;

    dr_GL_Bind(particletexture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	dr_VectorScale (vup, 1.5, up);
	dr_VectorScale (vright, 1.5, right);

    frametime = cltime - cloldtime;
	time3 = frametime * 15;
	time2 = frametime * 10; // 15;
	time1 = frametime * 5;
	grav = frametime * sv_gravity->value * 0.05;
	dvel = 4*frametime;
	
	for ( ;; ) 
	{
		kill = (*active_particles);
		if (kill && kill->die < cltime)
		{
			(*active_particles) = kill->next;
			kill->next = (*free_particles);
			(*free_particles) = kill;
			continue;
		}
		break;
	}

	for (p=(*active_particles) ; p ; p=p->next)
	{
		for ( ;; )
		{
			kill = p->next;
			if (kill && kill->die < cltime)
			{
				p->next = kill->next;
				kill->next = (*free_particles);
				(*free_particles) = kill;
				continue;
			}
			break;
		}
        
		glDisable(GL_TEXTURE_2D) ;

		glEnable(GL_BLEND) ;
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
		glDepthMask( 0 );
        glColor4f (0.0f,0.0f,0.0f,sketch_lalpha.value);
		glLineWidth(sketch_lthickness.value) ;

		//Draw our Shape...
		glBegin(GL_LINES);
		for (j=0; j < sketch_lnumberparticles.value; j++)
		{
			glVertex3f (p->org[0] + up[0] + randFloat(0.0f, 0.5f),
				        p->org[1] + up[1] + randFloat(0.0f, 0.5f),
						p->org[2] + up[2] + randFloat(0.0f, 0.5f));
			glVertex3f (p->org[0] + right[0] + randFloat(0.0f, 0.5f),
						p->org[1] + right[1] + randFloat(0.0f, 0.5f), 
						p->org[2] + right[2] + randFloat(0.0f, 0.5f));

			glVertex3f (p->org[0] + up[0] + right[0] + randFloat(0.0f, 0.5f), 
						p->org[1] + up[1] + right[1] + randFloat(0.0f, 0.5f), 
						p->org[2] + up[2] + right[2] + randFloat(0.0f, 0.5f));
			glVertex3f (p->org[0] + randFloat(0.0f, 0.5f),
						p->org[1] + randFloat(0.0f, 0.5f),
						p->org[2] + randFloat(0.0f, 0.5f));
		}
		glEnd() ;

		glLineWidth(1.0) ;
		glDepthMask (1);
		glDisable(GL_BLEND) ;

		glEnable(GL_TEXTURE_2D) ;

        p->org[0] += p->vel[0]*frametime;
		p->org[1] += p->vel[1]*frametime;
		p->org[2] += p->vel[2]*frametime;

		switch (p->type)
		{
		case pt_static:
			break;
		case pt_fire:
			p->ramp += time1;
			if (p->ramp >= 6)
				p->die = -1;
			else
				p->color = ramp3[(int)p->ramp];
			p->vel[2] += grav;
			break;

		case pt_explode:
			p->ramp += time2;
			if (p->ramp >=8)
				p->die = -1;
			else
				p->color = ramp1[(int)p->ramp];
			for (i=0 ; i<3 ; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_explode2:
			p->ramp += time3;
			if (p->ramp >=8)
				p->die = -1;
			else
				p->color = ramp2[(int)p->ramp];
			for (i=0 ; i<3 ; i++)
				p->vel[i] -= p->vel[i]*frametime;
			p->vel[2] -= grav;
			break;

		case pt_blob:
			for (i=0 ; i<3 ; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_blob2:
			for (i=0 ; i<2 ; i++)
				p->vel[i] -= p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_grav:

        case pt_slowgrav:
			p->vel[2] -= grav;
			break;
		}
        
	}

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

}






/*
=============
EmitWaterPolys

Does a water warp on the pre-fragmented glpoly_t chain
=============
*/
EXPORT void EmitWaterPolys (msurface_t *fa, double realtime)
{
	glpoly_t	*p;
	float		*v;
	int			i;
	int			j;
	float		s, t, os, ot;

	glDisable(GL_TEXTURE_2D) ;
	glEnable(GL_BLEND) ;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
	glDepthMask( 1 );
	glLineWidth(sketch_lthickness.value) ;

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f) ;

	for (p=fa->polys ; p ; p=p->next)
	{
		glBegin (GL_POLYGON);
		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			glVertex3fv (v);
		}
		glEnd ();
	}

	glDepthMask( 0 );

	for (p=fa->polys ; p ; p=p->next)
	{
		//glColor4f (0.64f,0.82f,0.95f,sketch_lalpha.value);
		glColor4f (0.0f,0.0f,0.0f,sketch_lalpha.value);

		glBegin (GL_LINE_LOOP);
		for (j=0; j < sketch_lnumberwater.value; j++)
		{
			for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
			{
				os = v[0];
				ot = v[1];

				s = 25 * turbsin[(int)((ot*0.125+realtime) * 600) & 255];
				s *= (1.0/64);

				t = 25 * turbsin[(int)((os*0.125+realtime) * 600) & 255];
				t *= (1.0/64);
			
				glVertex3f (v[0]+s+randFloat(-2.0,2.0), v[1]+t+randFloat(-2.0,2.0), v[2]+randFloat(-4.0,4.0)) ; //Raise it up and set it off to the 'right'
			}
		}
		glEnd ();

	}

	glLineWidth(1.0) ;
	glDepthMask (1);
	glDisable(GL_BLEND) ;
	glEnable(GL_TEXTURE_2D) ;
}




/*
=============
EmitSkyPolys
=============
*/
EXPORT void EmitSkyPolys (msurface_t *fa, float speedscale, vec3_t r_origin)
{
	glpoly_t	*p;
	float		*v;
	int			i;
	float	s, t;
	vec3_t	dir;
	float	length;

	glDisable(GL_TEXTURE_2D) ;
	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f) ;
	for (p=fa->polys ; p ; p=p->next)
	{
		glBegin (GL_POLYGON);
		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			VectorSubtract (v, r_origin, dir);
			dir[2] *= 3;	// flatten the sphere

			length = dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2];
			length = sqrt (length);
			length = 6*63/length;

			dir[0] *= length;
			dir[1] *= length;

			s = (speedscale + dir[0]) * (1.0/128);
			t = (speedscale + dir[1]) * (1.0/128);

			glTexCoord2f (s, t);
			glVertex3fv (v);


		}
		glEnd ();
	}
/*
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f) ;
	for (p=fa->polys ; p ; p=p->next)
	{
		glBegin (GL_LINES);
		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			VectorSubtract (v, r_origin, dir);
			dir[2] *= 3;	// flatten the sphere

			length = dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2];
			length = sqrt (length);
			length = 6*63/length;

			dir[0] *= length;
			dir[1] *= length;

			s = (speedscale + dir[0]) * (1.0/128);
			t = (speedscale + dir[1]) * (1.0/128);

			glTexCoord2f (s, t);
			glVertex3fv (v);


		}
		glEnd ();
	}
*/
	glEnable(GL_TEXTURE_2D) ;

}
