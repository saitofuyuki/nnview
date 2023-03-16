/*
 * Ncview by David W. Pierce.  A visual netCDF file viewer.
 * Copyright (C) 1993 through 2010 David W. Pierce
 *
 * This program  is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 3, as 
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License, version 3, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * David W. Pierce
 * 6259 Caminito Carrena
 * San Diego, CA  92122
 * pierce@cirrus.ucsd.edu
 */

/*
 * Normalization functions are extracted from interface/cbar.c and src/util.c.
 * More transformation are introduced.
 *
 * Created: Mar 16 2023
 * Maintainer: SAITO Fuyuki
 * Copyright (C) 2023
 *            Japan Agency for Marine-Earth Science and Technology
 */

#include "ncview.includes.h"
#include "ncview.defines.h"
#include "ncview.protos.h"

float
normalize_f (float data, const int transform)
{
  /*
   * This might cause problems.  It is at odds with what
   * the manual claims--at least for Ultrix--but works,
   * whereas what the manual claims works, doesn't!
   */
  const float half = 0.5F;
  float w, s;

  switch (transform)
    {
    case TRANSFORM_NONE:
      break;
    case TRANSFORM_LOW2:
      data = sqrtf (data);
      data = sqrtf (data);
      break;
    case TRANSFORM_LOW1:
      data = sqrtf (data);
      break;
    case TRANSFORM_HI1:
      data = data * data;
      break;
    case TRANSFORM_HI2:
      data = data * data;
      data = data * data;
      break;
    case TRANSFORM_ABSLOW1:
      w = (data - half) / half;
      s = copysignf(half, w);
      w = fabsf(w);
      w = sqrtf(w);
      data = half + s * w;
      break;
    case TRANSFORM_ABSLOW2:
      w = (data - half) / half;
      s = copysignf(half, w);
      w = fabsf(w);
      w = sqrtf(w);
      w = sqrtf(w);
      data = half + s * w;
      break;
    case TRANSFORM_ABSHI1:
      w = (data - half) / half;
      s = copysign(half, w);
      w = w * w;
      data = half + s * w;
      break;
    case TRANSFORM_ABSHI2:
      w = (data - half) / half;
      s = copysign(half, w);
      w = w * w;
      w = w * w;
      data = half + s * w;
      break;
    }
  return data;
}
double
normalize_d (double data, const int transform)
{
  /*
   * This might cause problems.  It is at odds with what
   * the manual claims--at least for Ultrix--but works,
   * whereas what the manual claims works, doesn't!
   */
  const float half = 0.5;
  double w, s;

  switch (transform)
    {
    case TRANSFORM_NONE:
      break;
    case TRANSFORM_LOW2:
      data = sqrt (data);
      data = sqrt (data);
      break;
    case TRANSFORM_LOW1:
      data = sqrt (data);
      break;
    case TRANSFORM_HI1:
      data = data * data;
      break;
    case TRANSFORM_HI2:
      data = data * data;
      data = data * data;
      break;
    case TRANSFORM_ABSLOW1:
      w = (data - half) / half;
      s = copysignf(half, w);
      w = fabs(w);
      w = sqrt(w);
      data = half + s * w;
      break;
    case TRANSFORM_ABSLOW2:
      w = (data - half) / half;
      s = copysignf(half, w);
      w = fabs(w);
      w = sqrt(w);
      w = sqrt(w);
      data = half + s * w;
      break;
    case TRANSFORM_ABSHI1:
      w = (data - half) / half;
      s = copysign(half, w);
      w = w * w;
      data = half + s * w;
      break;
    case TRANSFORM_ABSHI2:
      w = (data - half) / half;
      s = copysign(half, w);
      w = w * w;
      w = w * w;
      data = half + s * w;
      break;
    }
  return data;
}
