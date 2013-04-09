#ifndef _dynamic_r_h_
#define _dynamic_r_h_


// windows stuff to export dll functions.
#ifdef _WIN32
#define EXPORT __declspec( dllexport )
#else
#define EXPORT /**/
#endif

#pragma warning( disable : 4305 ) // shut up compiler -- trunc from double to float.



// only non-export -- used internally.
mspriteframe_t *R_GetSpriteFrame (entity_t *currententity, double cltime);



//
// All these functions are exported to Quake.
//
// When Quake loads the dll, it should call GetProcAddress() for all of these.
//
// IMMEDIATELY After that, it should call all the dr_Set functions, setting these
// functions to valid things.  If it doesn't, the next time Quake tries to draw
// the screen, the DLL will try to execute code at NULL (addr 0).
//


EXPORT void dr_Set_ConPrintf( void (*f)( char * fmt, ... ) );
EXPORT void dr_Set_VectorMA( void (*f)( vec3_t, float, vec3_t, vec3_t ) );
EXPORT void dr_Set_GL_Bind( void (*f)( int ) );
EXPORT void dr_Set_GL_DisableMultitexture( void (*f)() );
EXPORT void dr_Set_GL_EnableMultitexture( void (*f)() );
EXPORT void dr_Set_AngleVectors( void (*f)( vec3_t, vec3_t, vec3_t, vec3_t ) );
EXPORT void dr_Set_R_TextureAnimation( texture_t * (*f)( texture_t * ) );
EXPORT void dr_Set_VectorScale( void (*f)( vec3_t, vec_t, vec3_t ) );

EXPORT void dr_Set_Cvar_RegisterVariable( void (*f)( cvar_t * ) );
EXPORT void dr_Set_Cvar_UnRegisterVariable( void (*f)( cvar_t * ) );



// IMMEDIATELY after that, it should call this Init function.


EXPORT void dr_Init();



// BEFORE removing the renderer, it should call this Shutdown function.


EXPORT void dr_Shutdown();




EXPORT void GL_SelectTexture (GLenum target, qboolean gl_mtexable, 
                       lpSelTexFUNC qglSelectTextureSGIS, int currenttexture,
                       int * cnttextures);

EXPORT void R_DrawSpriteModel (entity_t *e, entity_t *currententity,
                        double cltime, vec3_t vup, vec3_t vright);
EXPORT void GL_DrawAliasFrame (aliashdr_t *paliashdr, int posenum, 
                        float * shadedots, float shadelight,
						entity_t * currententity );//JG
EXPORT void GL_DrawAliasShadow (aliashdr_t *paliashdr, entity_t * currententity,
                         int posenum, vec3_t shadevector,
                         vec3_t lightspot);

EXPORT void R_PolyBlend (float * v_blend, cvar_t * gl_polyblend);
EXPORT void R_DrawSequentialPoly (msurface_t *s, int lightmap_textures,
                           qboolean * lightmap_modified, 
                           glRect_t * lightmap_rectchange, byte * lightmaps,
                           int lightmap_bytes, int solidskytexture, 
                           float * speedscale, int alphaskytexture, 
                           qboolean gl_mtexable, int currenttexture,
                           int * cnttextures, double realtime, 
                           lpMTexFUNC qglMTexCoord2fSGIS, int gl_lightmap_format,
                           lpSelTexFUNC qglSelectTextureSGIS, vec3_t r_origin );
EXPORT void DrawGLWaterPoly (glpoly_t *p, double realtime);
EXPORT void DrawGLWaterPolyLightmap (glpoly_t *p, double realtime);
EXPORT void DrawGLPoly (glpoly_t *p);
EXPORT void R_BlendLightmaps ( glpoly_t ** lightmap_polys, int lightmap_textures,
                       qboolean * lightmap_modified, glRect_t * lightmap_rectchange,
                       byte * lightmaps, int lightmap_bytes, int gl_lightmap_format,
                       cvar_t * r_lightmap, cvar_t * gl_texsort, cvar_t * r_fullbright,
                       double realtime );
EXPORT void R_DrawParticles (particle_t ** active_particles, particle_t ** free_particles,
                      int * ramp1, int * ramp2, int * ramp3, cvar_t * sv_gravity,
                      double cloldtime, double cltime, int particletexture,
                      vec3_t vright, vec3_t vup, unsigned int * d_8to24table,
                      vec3_t vpn, vec3_t r_origin);
EXPORT void EmitWaterPolys (msurface_t *fa, double realtime);
EXPORT void EmitSkyPolys (msurface_t *fa, float speedscale, vec3_t r_origin);




#endif /* _dynamic_r_h_ */
