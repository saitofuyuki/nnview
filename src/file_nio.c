/*
 * TOUZA/Nio interface for ncview
 * Created: Dec 12 2022
 * Maintainer: SAITO Fuyuki
 *
 * Copyright (C) 2022-2023
 *            Japan Agency for Marine-Earth Science and Technology
 *
 * This program  is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
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
 */

#include "ncview.includes.h"
#include "ncview.defines.h"
#include "ncview.protos.h"

#ifdef HAVE_UDUNITS2
#include "utCalendar2_cal.h"
#endif

#include "touza_nio_interface.h"

static int   file_type;
extern NCVar *variables;
extern Options options;

static int init = 0;

const int lgroup = 64;
const int lvar   = 32;
const int ldim   = 4;
const int litem  = 16;

const int dim_rec = 0;
const int dim_ofs = 1;

#define NAME_REC "rec"

int nio_inq_varid_grp (const int fileid, const char *varname,
                       int * const groupid, int * const varid);


int
nio_fi_confirm (const char *name)
{
  int krect;
  int jerr;
  int mode = MODE_DEEPEST;
  int levv = 0;

  if (init == 0)
    {
      if (options.debug) levv = +99;
      jerr = tnb_init(levv, mode);
      if (jerr == 0) init = init + 1;
    }
  if (init > 0)
    {
      krect = tnb_file_is_nio(name);
      if (options.debug) fprintf(stderr, "nio_fi_confirm: krect=%d\n", krect);
      if (krect < 0) return (FALSE);
      return (TRUE);
    }
  return(FALSE);
}

int nio_fi_initialize(const char *name)
{
  int fileid, ierr;

  fileid = tnb_file_open(name, 0);
  if (fileid < 0)
    {
      fprintf(stderr, "nio_fi_initialize: can't properly open file %s\n", name);
      exit(-1);
    }
  return (fileid);
}

void
nio_fi_list_vars_inner (Stringlist **ret_val,
                        int fileid, int gid, char *groupname)
{
  int  jv, n_vars;
  char var_name[lvar+1];
  char grp_var_name[lgroup + lvar + 1];
  int  jerr;
  int  jd, nd, eff_ndims;
  size_t *size, total_size;
  int  nrecs;

  n_vars = tnb_group_vars(fileid, gid);

  if (n_vars < 0)
    {
      fprintf(stderr, "nio_fi_list_vars: error on query nvars %d %d\n", fileid, gid);
      exit(-1);
    }

  nrecs = tnb_group_recs(fileid, gid);
  if (nrecs < 0)
    {
      fprintf(stderr, "nio_fi_list_vars: error on query nrecs %d %d\n", fileid, gid);
      exit(-1);
    }

  for (jv = 0; jv < n_vars; jv++)
    {
      jerr = tnb_var_name(var_name, fileid, gid, jv);
      if (jerr != 0) exit(-1);
      grp_var_name[0] = '\0';
      if ((strlen(groupname) == 0) || ((strlen(groupname) == 1) && (groupname[0] == '/')))
        strcpy (grp_var_name, var_name);
      else
        {
          strcat (grp_var_name, groupname);
          strcat (grp_var_name, "/" );
          strcat (grp_var_name, var_name);
        }
      if (options.debug)
        fprintf(stderr,
                "nnview/nio_fi_list_vars_inner: "
                "checking to see if a displayable var: >%s<\n", grp_var_name);
      nd = nio_fi_var_dimsize(&size, fileid, gid, jv);
      total_size = 1L;
      eff_ndims  = 0;
      for (jd = 0; jd < nd; jd++)
        {
          total_size *= size[jd];
          if (size[jd] > 1) eff_ndims++;
        }
      if ((total_size > 1L) && (nd >= 1))
        {
          if (! (options.no_1d_vars && (eff_ndims == 1)))
            {
              if (options.debug)
                {
                  fprintf(stderr,
                          "nio_fi_list_vars_inner: YES, is a displayable var: >%s< ndims=%d sizes=",
                          grp_var_name, nd);
                  for (jd = 0; jd < nd; jd++)
                    fprintf(stderr, "%ld ", size[jd]);
                  fprintf(stderr, "\n");
                }
              stringlist_add_string (ret_val, grp_var_name, NULL, SLTYPE_NULL);
            }
        }
      else
        if (options.debug) fprintf(stderr,
                                   "nio_fi_list_vars_inner: NO, is size 1 so not displayable: >%s<\n",
                                   grp_var_name);
    }
}

Stringlist *nio_fi_list_vars(const int fileid)
{
  Stringlist *retval = NULL;
  int jgrp, n_groups;
  char groupname[lgroup+1];

  if (options.debug)
    fprintf(stderr, "nio_fi_list_vars: entering for file %d\n", fileid);

  n_groups = tnb_file_groups(fileid);
  if (n_groups < 0)
    {
      fprintf(stderr, "nio_fi_list_vars: error on tnb_file_groups %d, %d\n", fileid, n_groups);
      exit (-1);
    }
  for (jgrp = 0; jgrp < n_groups; jgrp++)
    {
      sprintf(groupname, "G%d", jgrp);
      nio_fi_list_vars_inner(&retval, fileid, jgrp, groupname);
    }

  if (options.debug)
    {
      fprintf (stderr,
               "nio_fi_list_vars: exiting with list of DISPLAYABLE vars in this file:\n");;
      stringlist_dump(retval);
    }
  return (retval);
}

size_t *nio_fi_var_size (const int fileid, const char * var_name)
{
  size_t *ret_val, dim_size;
  int gid, vid;
  int ierr;
  int jd, n_dims, jco, nco;
  int dims;

  ret_val = NULL;

  ierr = nio_inq_varid_grp (fileid, var_name, &gid, &vid);
  if (ierr != 0)
    {
      fprintf(stderr, "Error in nio_fi_var_size: could not find var named \"%s\" in file!\n",
              var_name);
      exit(-1);
    }
  nco = tnb_var_nco(fileid, gid, vid);
  n_dims = nco + 1;
  if (nco < 0)
    {
      fprintf (stderr, "Error in nio_fi_var_size: %d %d %d\n", fileid, gid, vid);
      exit (-1);
    }
  ret_val = (size_t *) malloc (n_dims * sizeof(size_t));
  for (jco = 0; jco < nco; jco++)
    {
      jd = jco + dim_ofs;
      dims = tnb_co_size(fileid, gid, vid, jco);
      ret_val[jd] = dims;
    }
  ret_val[dim_rec] = tnb_group_recs(fileid, gid);

  return (ret_val);
}

int nio_fi_var_dimsize (size_t **size, const int fileid, const int gid, const int vid)
{
  int jd, n_dims, jco, nco;
  int dims;
  int nrecs;
  size_t *ret_val;

  nco = tnb_var_nco(fileid, gid, vid);
  n_dims = nco + 1;
  if (nco < 0 || n_dims > ldim)
    {
      fprintf (stderr, "Error in nio_fi_var_dimsize: %d %d %d\n", fileid, gid, vid);
      exit (-1);
    }
  nrecs = tnb_group_recs(fileid, gid);
  if (nrecs < 0)
    {
      fprintf (stderr, "Error in nio_fi_var_dimsize: %d %d %d\n", fileid, gid, vid);
      exit (-1);
    }
  ret_val = (size_t *) malloc (n_dims * sizeof(size_t));
  for (jco = 0; jco < nco; jco++)
    {
      jd = jco + dim_ofs;
      dims = tnb_co_size(fileid, gid, vid, jco);
      ret_val[jd] = dims;
    }
  ret_val[dim_rec] = nrecs;
  *size = ret_val;
  return (n_dims);
}

void nio_fill_aux_data(const int fileid, const char *var_name, FDBlist *fdb)
{
  int vid, gid;
  int ierr;
  char *rec_unit = NULL;

  ierr = nio_inq_varid_grp (fileid, var_name, &gid, &vid);
  if (ierr != 0)
    {
      fprintf(stderr, "Error in nio_fill_aux_data: could not find var named \"%s\" in file!\n",
              var_name);
      exit(-1);
    }
  if (ierr == 0)
    {
      rec_unit = (char *) malloc (litem * sizeof(char));
      ierr = tnb_get_attr(rec_unit, "UTIM", fileid, gid, -1, -1);
    }
  if (ierr == 0)
    {
      fdb->recdim_units = rec_unit;
    }
  else
    {
      fdb->recdim_units = NULL;
    }
}

int nio_fi_n_dims (const int fileid, const char *var_name)
{
  int vid, gid;
  int ierr;
  char *rec_unit = NULL;
  int n_dims;

  ierr = nio_inq_varid_grp (fileid, var_name, &gid, &vid);
  if (ierr != 0)
    {
      fprintf(stderr, "Error in nio_fi_n_dims: could not find var named \"%s\" in file!\n",
              var_name);
      exit(-1);
    }
  n_dims = tnb_var_nco(fileid, gid, vid);
  return n_dims + 1;
}

int nio_has_dim_values (const int fileid, const char *dim_name)
{
  /* reserved */
  return(FALSE);
}

void nio_fill_value(const int file_id, const char *var_name, float *v)
{
  /* reserved */

  *v = NC_FILL_FLOAT;

  if (options.debug)
    fprintf(stderr, "setting fillvalue to default for var type=%g\n", *v);
}

char *nio_get_char_att(const int fileid,
                       const char *var_name, char * const att_nam)
{
  /* reserved */
  return(NULL);
}

char *nio_dim_id_to_name(const int fileid, const char *var_name, const int dim_id)
{
  int ierr;
  int gid, vid;
  char *dim_name;

  ierr = nio_inq_varid_grp (fileid, var_name, &gid, &vid);

  if (ierr != 0)
    {
      fprintf(stderr, "Error in nio_dim_id_to_name: could not find var named \"%s\" in file!\n",
              var_name);
      exit(-1);
    }
  dim_name = (char *) malloc(litem + 1);
  if (dim_id == dim_rec)
    {
      ierr = 0;
      strcpy(dim_name, NAME_REC);
    }
  else
    {
      ierr = tnb_co_name(dim_name, fileid, gid, vid, dim_id - dim_ofs);
    }
  if (ierr != 0)
    {
      fprintf (stderr, "ncview: nio_dim_id_to_name: error on ");
      fprintf (stderr, "tnb_co_name call.  Variable=%s\n", var_name);
      exit(-1);
    }
  /* printf("dim_name: %s\n", dim_name); */
  return(dim_name);
}

int nio_dim_name_to_id(const int fileid, const char *var_name,
                       const char *dim_name)
{
  int ierr;
  int gid, vid;
  int cidx;

  ierr = nio_inq_varid_grp (fileid, var_name, &gid, &vid);

  if (ierr != 0)
    {
      fprintf(stderr, "Error in nio_dim_id_to_name: could not find var named \"%s\" in file!\n",
              var_name);
      exit(-1);
    }

  if (strcmp(dim_name, NAME_REC) == 0)
    {
      cidx = dim_rec;
    }
  else
    {
      cidx = tnb_co_idx(fileid, gid, vid, dim_name);
      if (cidx >= 0) cidx = cidx + dim_ofs;
    }

  return(cidx);
}

char *nio_dim_longname(const int fileid, char * const dim_name)
{
  return(dim_name);
}

char *nio_dim_units(const int fileid, const char *dim_name)
{
  /* reserved */
  return(NULL);
}

char *nio_dim_calendar(const int fileid, const char *dim_name)
{
  /* reserved */
  return(NULL);
}

nc_type nio_dim_value(int fileid, char *dim_name, size_t place,
        double *ret_val_double, char *ret_val_char, size_t virt_place,
        int *return_has_bounds, double *return_bounds_min, double *return_bounds_max)
{
  nc_type ret_type;

  /* return logical index as physical coordinate */
  *ret_val_double = (double)virt_place;
  *return_has_bounds = 0;
  return(NC_DOUBLE);
}

Stringlist *nio_scannable_dims(const int fileid, const char *var_name)
{
  /* reserved */
  Stringlist *dimlist = NULL;
  int gid, vid;
  int ierr;
  int dims;
  int jd, n_dims;
  char dim_name[litem + 16];

  ierr = nio_inq_varid_grp(fileid, var_name, &gid, &vid);
  if (ierr != 0)
    {
      fprintf(stderr, "Error in nio_scannable_dims: could not find var named \"%s\" in file!\n",
              var_name);
      exit(-1);
    }
  n_dims = tnb_var_nco(fileid, gid, vid);
  if (n_dims < 0)
    {
      fprintf (stderr, "Error in nio_scannable_dims: %d %d %d\n", fileid, gid, vid);
      exit(-1);
    }
  stringlist_add_string(&dimlist, NAME_REC, NULL, SLTYPE_NULL);
  for(jd = 0; jd < n_dims; jd++)
    {
      dims = tnb_co_size(fileid, gid, vid, jd);
      if(dims > 1)
        {
          ierr = tnb_co_name(dim_name, fileid, gid, vid, jd);
          stringlist_add_string(&dimlist, dim_name, NULL, SLTYPE_NULL);
        }
    }

  return(dimlist);
}

void nio_fi_get_data(const int fileid, const char *var_name,
                     const size_t *start_pos, const size_t *count,
                     float *data, NetCDFOptions *aux_data)
{
  int ierr, jerr;
  int gid, vid;
  int jd, n_dims;
  size_t tot_size, sp_size;
  const size_t *sp_start = & start_pos[1];
  const size_t *sp_count = & count[1];
  int jr;
  int rec;
  float vmiss;
  int j;

  ierr = nio_inq_varid_grp(fileid, var_name, &gid, &vid);
  if (ierr != 0)
    {
      fprintf(stderr, "Error in nio_fi_get_data: could not find var named \"%s\" in file!\n",
              var_name);
      exit(-1);
    }
  n_dims = tnb_var_nco(fileid, gid, vid);
  if (n_dims < 0)
    {
      fprintf (stderr, "Error in nio_fi_get_data:: %d %d %d\n", fileid, gid, vid);
      exit(-1);
    }

  n_dims++;
  tot_size = 1L;
  for (jd = 0; jd < n_dims; jd++)
    {
      tot_size *= count[jd];
    }
  sp_size = tot_size / count[dim_rec];

  if (options.debug)
    {
      fprintf(stderr, "About to call nio_get_var_f on variable %s\n",
              var_name );
      fprintf(stderr, "Index, start, count:\n");
      for(jd = 0; jd < n_dims; jd++)
        fprintf(stderr, "[%d]: %ld %ld\n", jd, start_pos[jd], count[jd]);
    }

  for (jr = 0; jr < count[dim_rec]; jr++)
    {
      rec = start_pos[dim_rec] + jr;
      jerr = tnb_get_attr_float(&vmiss, "MISS", fileid, gid, vid, rec);
      if (ierr == 0)
        {
          ierr = tnb_var_read_float(&data[sp_size * jr], rec, sp_start, sp_count,
                                    fileid, gid, vid);
        }
      if (jerr == 0)
        {
          for (j = 0L; j < sp_size; j++)
            {
              if (data[j] == vmiss) data[j] = NC_FILL_FLOAT;
            }
        }
    }

}

char *nio_title(const int fileid)
{
  /* reserved */
  return(NULL);
}

char *nio_var_units(const int fileid, const char *var_name)
{
  /* reserved */
  return(NULL);
}
char *nio_long_var_name(const int fileid, const char *var_name)
{
  /* reserved */
  return(NULL);
}
int nio_fi_recdim_id(const int fileid)
{
  return(dim_rec);
}
size_t
nio_fi_check_has_grown(char *file, char *var_name, size_t *ref_sizes, int dim_id)
{
  /* reservd */
  size_t ret_val;
  ret_val = 0;
  return(ret_val);
}

int nio_inq_varid_grp (const int fileid, const char *varname,
                       int * const groupid, int * const varid)
{
  const char *vp, *gp;
  int vid, gid;
  int ierr = 0;
  long int gtmp;

  vp = strchr(varname, '/');

  if (vp == NULL)
    {
      /* no group */
      gid = 0;
      vid = tnb_var_id(fileid, gid, varname);
    }
  else
    {
      gp = varname; gp++; /* skip 'G' */
      gtmp = strtol(gp, NULL, 10);
      gid = (int) gtmp;
      vp++;
      /* printf("gid, vp: %d %s\n", gid, vp); */
      vid = tnb_var_id(fileid, gid, vp);
    }
  *groupid = gid;
  *varid   = vid;
  /* if (options.debug) printf("nnview/nio_inq_varid_grp: %s %d %d\n", */
  /*                           varname, gid, vid); */

  if (vid < 0) return (vid);
  return (ierr);
}

char *
nio_fi_att_string(int fileid, char *var_name)
{
  size_t len, retval_len=10000;
  char *ret_string;
  char aitem[16];
  char attr[litem+1];
  int gid, vid;
  int ierr;
  int ji;

  ret_string = (char *) malloc (retval_len);
  snprintf (ret_string, retval_len,
            "Attributes for variable %s:\n------------------------------\n",
            var_name );
  ret_string[retval_len-1] = '\0';

  ierr = nio_inq_varid_grp (fileid, var_name, &gid, &vid);
  if (ierr != 0)
    {
      fprintf(stderr, "Error in nio_fi_att_string: could not find var named \"%s\" in file!\n",
              var_name);
      exit(-1);
    }
  for (ji = 1; ji <= TNB_HEADER_ITEMS; ji++)
    {
      ierr = tnb_get_attr_name(aitem, ji);
      ierr = tnb_get_attr_byid(attr, ji, fileid, gid, vid, 0);
      if (strcmp(attr, "") != 0)
        {
          safe_strcat (ret_string, retval_len, aitem);
          safe_strcat (ret_string, retval_len, ": ");
          safe_strcat (ret_string, retval_len, attr);
          safe_strcat (ret_string, retval_len, "\n");
        }
    }
  return (ret_string);
}
