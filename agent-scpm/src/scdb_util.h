/*
 * $Id$
 *
 *
 * Project     :  SCPM (SuSE Configuration Profile Management)
 * Module      :  SCDB manipulation utility
 * File        :  scdb_util.hh
 * Description :  Class containing utility functions for SCDB
 * Author      :  Joachim Gleissner <jg@suse.de>
 *
 * Copyright 2002 SuSE Linux AG
 *
 * Released under the terms of the GNU General Public License
 * (see file COPYRIGHT in project root directory).
 *
 */
#ifndef SCDB_UTIL_H
#define SCDB_UTIL_H
#include <scdb.h>
#include <string>
#include <vector>

class SCDBUtil
{
public:
  SCDBUtil() { db = SCDB::GetHandle(); };

  typedef struct operation_failed
  {
  public:
    operation_failed( string r ) { reason = r; };
    string reason;
  };

  // creates a new resource and adds it to all profiles
  void CreateResource( string resource_name, string resource_type, 
		       string profile="all", bool skip_update=false )
    throw( operation_failed );
  // function to remove a Resource from the resource db and from profiles
  void DropResource( string resource_name, string resource_type, bool full=true )
    throw( operation_failed );
  // removes a resource from one or all profiles
  void RemoveResourceFromProfile( string resource_name, string resource_type,
				  string profile="all" )
    throw( operation_failed );
  // adds a resoure to one or all profiles
  void AddResourceToProfile( string resource_name, string resource_type,
			     string profile="all" )
    throw( operation_failed );
  // make a resource static in multiple profiles
  void BindResource( string resource_name, string resource_type,
		     string src_profile="none", string dest_profile="all" )
    throw( operation_failed );
  // release a bind resource from one or all profiles
  void ReleaseResource( string resource_name, string resource_type,
			string profile="all" )
    throw( operation_failed );
  // checks for resources which are the same in ALL profiles
  void FindUnusedResources( vector<string> &resource_names, vector<string> &resource_types )
    throw( operation_failed );
  // checks for resources which exist but are not in db
  void FindUnhandledResources( vector<string> &resource_names, vector<string> &resource_types )
    throw( operation_failed );
  // checks for resources which has been removed from the system
  void FindDeletedResources( vector<string> &resource_names, vector<string> &resource_types )
    throw( operation_failed );
  // removes all resources from resource db and profiles that are found by FindUnusedResources
  void MinimizeDB( bool full=true ) throw( operation_failed );
  // checks for available resources that are actually not in the db and adds them
  void MaximizeDB( bool add_to_profiles=true ) throw( operation_failed );
  // rebuilds dependencies for all resources
  void RebuildDeps( ) throw( operation_failed );
  

private:
  SCDB *db;
};

#endif
