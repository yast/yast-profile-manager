/*
 * $Id$
 *
 *
 * Project     :  SCPM (SuSE Configuration Profile Management)
 * Module      :  SCPM main engine
 * File        :  scpm.cc
 * Description :  defines the SCPM interface
 * Author      :  Joachim Gleissner <jg@suse.de>
 *
 * Copyright 2001 SuSE GmbH
 *           2002 SuSE Linux AG
 *
 * Released under the terms of the GNU General Public License
 * (see file COPYRIGHT in project root directory).
 *
 */
#ifndef SCPM_H
#define SCPM_H

#include <string>
#include <vector>
#include "scpm_types.h"
#include "scdb.h"
//#include <scdb.h>

class SCPM
{
public:

  SCPM( int options=0, ostream &info_out=cout, ostream &hash_out=cout );
  ~SCPM();

  bool Enable( );
  bool Disable( );

  bool Active( string &profile );
  bool List( vector<string> &profiles );

  bool Add( string profile, bool auto_switch=false );
  bool Copy( string source_profile, string profile );
  bool Create( string profile, string source_profile );
  bool Delete( string profile );
  bool Rename( string profile, string newprofile );
  bool Reload( );
  bool PrepareSwitch( string profile, switch_info_t &switch_info );
  bool Switch( switch_info_t &switch_info );
  bool Modify( string profile, string action );
  bool Set( string command, string argument, string profile="" );
  bool Get( string command, string &result, string profile="" );
  bool Save( switch_info_t &switch_info );
  bool Status( scpm_status_t &status );

  bool SetResourceSet( string set );
  bool GetResourceSet( string &set );
  bool ListResourceSets( vector<string> &predefined, vector<string> &individual );
  bool CopyResourceSet( string set, string newset );
  bool DeleteResourceSet( string set );

private:

};

#endif
