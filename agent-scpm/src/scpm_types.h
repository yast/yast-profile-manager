/*
 * $Id$
 *
 * Type definitions for the SCPM interface.
 *
 * Copyright 2001 SuSE GbmH
 *
 * Author: Joachim Gleissner <jg@suse.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef SCPM_TYPES_H
#define SCPM_TYPES_H

#include <string>
#include <vector>


typedef struct resource_info_t {
  string resource_name;
  string resource_type;
  bool   is_new;
  bool   is_deleted;
  bool   save;
} ;

typedef struct switch_info_t {
  bool profile_modified;
  string profile_name;
  vector<resource_info_t> modified_resources;
} ;

typedef struct scpm_status_t {
  bool initialized;
  bool enabled;
  bool db_loaded;
  bool db_uptodate;
  string active_profile;
  string scpm_version;
  string db_version;
  string db_format;
  string db_format_version;
} ;

extern char *scpm_error;

const int scpm_flag_force           = 1;
const int scpm_flag_skip            = 2;
const int scpm_flag_quiet           = 4;
const int scpm_flag_verbose         = 8;
const int scpm_flag_hash            = 16;
const int scpm_flag_debug           = 32;
const int scpm_flag_boot            = 64;
const int scpm_flag_exit_on_warning = 128;
const int scpm_flag_skip_load       = 256;
const int scpm_flag_force_db        = 512;

#endif
