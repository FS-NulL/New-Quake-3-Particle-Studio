#ifndef __ASE_FORMAT__
#define __ASE_FORMAT__

#define ASE_HEADER "*3DSMAX_ASCIIEXPORT	200\n*COMMENT \"AsciiExport Version  2.00 - Mon Nov 26 15:56:44 2007\"\n*SCENE {\n	*SCENE_FILENAME \"ps2 export\"\n	*SCENE_FIRSTFRAME 0\n	*SCENE_LASTFRAME 100\n	*SCENE_FRAMESPEED 30\n	*SCENE_TICKSPERFRAME 160\n	*SCENE_BACKGROUND_STATIC 0.0000	0.0000	0.0000\n	*SCENE_AMBIENT_STATIC 0.0000	0.0000	0.0000\n}\n*MATERIAL_LIST {\n	*MATERIAL_COUNT ";
// Follow with material count
#define ASE_MATERIAL1 "\n\t*MATERIAL ";		// Follow with material number
#define ASE_MATERIAL2 " {\n		*MATERIAL_NAME \"textures/"; // Follow with texture name
#define ASE_MATERIAL3 "\"\n	}";			// End of material definition
#define ASE_MATERIALE "\n}";

#define ASE_GEOM1 "\n*GEOMOBJECT {\n	*NODE_NAME \"ps_system_0";			// follow with system number
#define ASE_GEOM2 "\"\n	*NODE_TM {\n		*NODE_NAME \"ps_system_0";	// follow with system number
#define ASE_GEOM3 "\"\n		*INHERIT_POS 0 0 0\n		*INHERIT_ROT 0 0 0\n		*INHERIT_SCL 0 0 0\n		*TM_ROW0 1.0000	0.0000	0.0000\n		*TM_ROW1 0.0000	1.0000	0.0000\n		*TM_ROW2 0.0000	0.0000	1.0000\n		*TM_ROW3 0.0000	0.0000	0.0000\n		*TM_POS 0.0000	0.0000	0.0000\n		*TM_ROTAXIS 0.0000	0.0000	0.0000\n		*TM_ROTANGLE 0.0000\n		*TM_SCALE 1.0000	1.0000	1.0000\n		*TM_SCALEAXIS 0.0000	0.0000	0.0000\n		*TM_SCALEAXISANG 0.0000\n	}\n	*MESH {\n		*TIMEVALUE 0\n		*MESH_NUMVERTEX 4\n		*MESH_NUMFACES 2\n		*MESH_VERTEX_LIST {";
// Follow by vertex list
#define ASE_GEOM4 "\n\t\t\t*MESH_VERTEX";
#define ASE_GEOM5 "\n}\n		*MESH_FACE_LIST {\n			*MESH_FACE    0:    A:   0 B:   1 C:  2 AB:    1 BC:    1 CA:    0	 *MESH_SMOOTHING 	*MESH_MTLID 0\n			*MESH_FACE    1:    A:  2 B:  3 C:   0  AB:    1 BC:    1 CA:    0	 *MESH_SMOOTHING 	*MESH_MTLID 0			\n		}\n		*MESH_NUMTVERTEX 4\n		*MESH_TVERTLIST {\n			*MESH_TVERT 0	0.0000	0.0000	0.0000\n			*MESH_TVERT 1	1.0000	0.0000	0.0000\n			*MESH_TVERT 2	1.0000	1.0000	0.0000\n			*MESH_TVERT 3	0.0000	1.0000	0.0000\n		}\n		*MESH_NUMTVFACES 2\n		*MESH_TFACELIST {\n			*MESH_TFACE 0	0	1	2\n			*MESH_TFACE 1	2	3	0\n		}\n		*MESH_NUMCVERTEX 0\n	}\n	*PROP_MOTIONBLUR 0\n	*PROP_CASTSHADOW 1\n	*PROP_RECVSHADOW 1\n	*MATERIAL_REF ";
//Follow by texture referance number
#define ASE_GEOM6 "\n}";

#endif


