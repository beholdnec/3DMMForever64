/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: brender.h 1.19 1995/08/31 16:36:12 sam Exp $
 * $Locker:  $
 *
 * Master include file for BRender
 *
 */
#ifndef _BRENDER_H_
#define _BRENDER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "compiler.h"

#ifndef _BRLIMITS_H_
#include "brlimits.h"
#endif

#ifndef _FIXED_H_
#include "fixed.h"
#endif

#ifndef _SCALAR_H_
#include "scalar.h"
#endif

#ifndef _COLOUR_H_
#include "colour.h"
#endif

#ifndef _MATRIX_H_
#include "matrix.h"
#endif

#ifndef _VECTOR_H_
#include "vector.h"
#endif

#ifndef _ANGLES_H_
#include "angles.h"
#endif

#ifndef _QUAT_H_
#include "quat.h"
#endif

#ifndef _TRANSFRM_H_
#include "transfrm.h"
#endif

#ifndef _ORDER_H_
#include "order.h"
#endif

#ifndef _PIXELMAP_H_
#include "pixelmap.h"
#endif

#ifndef _BRFONT_H_
#include "brfont.h"
#endif

#ifndef _MATERIAL_H_
#include "material.h"
#endif

#ifndef _MODEL_H_
#include "model.h"
#endif

#ifndef _ACTOR_H_
#include "actor.h"
#endif

#ifndef _CAMERA_H_
#include "camera.h"
#endif

#ifndef _LIGHT_H_
#include "light.h"
#endif

#ifndef _POOL_H_
#include "pool.h"
#endif

#ifndef _CUSTSUPT_H_
#include "custsupt.h"
#endif

#ifndef _BRFILE_H_
#include "brfile.h"
#endif

#ifndef _BRDIAG_H_
#include "brdiag.h"
#endif

#ifndef _BRMEM_H_
#include "brmem.h"
#endif

#ifndef _BRERR_H_
#include "brerr.h"
#endif

#ifndef _FMT_H_
#include "fmt.h"
#endif

    /*
     * Function prototypes
     */

#ifndef _FWPROTO_H_
#include "fwproto.h"
#endif

#ifndef _ZBPROTO_H_
#include "zbproto.h"
#endif

#ifndef _ZSPROTO_H_
#include "zsproto.h"
#endif

    /*
     * Inline functions
     */

#ifndef _NO_PROTOTYPES
#ifndef _NO_VECTOR_MACROS
#ifndef _VECFNS_H_
#include "vecfns.h"
#endif
#endif
#endif

#include "compend.h"

#ifdef __cplusplus
};
#endif
#endif


