/*
 * $Id$
 *
 *
 * Project     :  SCPM (SuSE Configuration Profile Management)
 * Module      :  SCPM database
 * File        :  scdb.hh
 * Description :  defines SCDB access and management functions
 * Author      :  Joachim Gleissner <jg@suse.de>
 *
 * Copyright 2002 SuSE Linux AG
 *
 * Released under the terms of the GNU General Public License
 * (see file COPYRIGHT in project root directory).
 *
 */
#ifndef SCDB_H
#define SCDB_H

#include <vector>
#include <string>
#include <iostream>

class SCDB
{
public:
  friend class SCPM;
  friend class SCDBUtil;


  typedef struct dbinfo_t {
    string version;
    string type;
    bool is_local;
  };

  typedef struct resource_dep_t {
    string name;
    string type;
  };

  typedef struct db_action_failed
  {
  public:
    db_action_failed( string r ) { reason = r; };
    string reason;
  };

  static SCDB * GetHandle( ) { return scdb_handle; };

  // scdb specific functions
  struct dbinfo_t GetDBInfo( );
  void LoadDB( )  throw( db_action_failed );
  void LoadDB( string dbfile )  throw( db_action_failed );
  void LoadDB( istream &input )  throw( db_action_failed );
  void SaveDB( )  throw( db_action_failed );
  void SaveDB( ostream &output )  throw( db_action_failed );
  void DropDB( );
  bool Modified( ) { return modified; };
  string GetSCDBVersion( );
  void   SetSCDBVersion( string version );
  string GetFormat( ) { return "xml"; };
  string GetFormatVersion( );

  // scdb global functions
  void SetStatusFlag( string status_flag, bool value ) throw( db_action_failed );
  bool GetStatusFlag( string status_flag ) throw( db_action_failed );
  void SetActiveProfile( string profile_name ) throw( db_action_failed );
  string GetActiveProfile( ) throw( db_action_failed );
  vector<string> GetProfiles( ) throw( db_action_failed );

  // resource functions
  void ResourceAdd( string resource_name, string resource_type )
    throw( db_action_failed );
  void ResourceAddType( string resource_type )
    throw( db_action_failed );
  void ResourceDelete( string resource_name, string resource_type )
    throw( db_action_failed );
  void ResourceAddDep( string resource_name, string resource_type, string dep_name, string dep_type )
    throw( db_action_failed );
  void ResourceCleanDeps( string resource_name, string resource_type )
    throw( db_action_failed );
  vector<resource_dep_t> ResourceGetDeps( string resource_name, string resource_type )
    throw( db_action_failed );
  vector<string> ResourceGetTypes( )
    throw( db_action_failed );
  vector<string> ResourceGetNames( string resource_type )
    throw( db_action_failed );
  void ResourceAddData( string resource_name, string resource_type,
			string data_entry, string data_value )
    throw( db_action_failed );
  void ResourceClearData( string resource_name, string resource_type )
    throw( db_action_failed );
  void ResourceGetData( string resource_name, string resource_type,
			vector<string> &data_entries, vector<string> &data_values )
    throw( db_action_failed );

  // profile functions
  bool ProfileExists( string profile_name )
    throw( db_action_failed );
  void ProfileAdd( string profile_name )
    throw( db_action_failed );
  void ProfileDelete( string profile_name )
    throw( db_action_failed );
  void ProfileAddResource( string profile_name, string resource_name, string resource_type )
    throw( db_action_failed );
  void ProfileDeleteResource( string profile_name, string resource_name, string resource_type )
    throw( db_action_failed );
  void ProfileClearResources( string profile_name, string resource_type="all" )
    throw( db_action_failed );
  bool ProfileGetFlag( string profile, string flag )
    throw( db_action_failed );
  void ProfileSetFlag( string profile, string flag, bool value )
    throw( db_action_failed );
  string ProfileGetKey( string profile, string key )
    throw( db_action_failed );
  void ProfileSetKey( string profile, string key, string value )
    throw( db_action_failed );
  string ProfileGetScript( string profile, string script_name )
    throw( db_action_failed );
  void ProfileSetScript( string profile, string script_name, string script_location )
    throw( db_action_failed );
  vector<string> ProfileGetResourceTypes( string profile )
    throw( db_action_failed );
  vector<string> ProfileGetResources( string profile, string resource_type )
    throw( db_action_failed );

  // generic access functions
  void Dump( ostream &output, string key="root" )
    throw( db_action_failed );
  void SetValue( string key, string value )
    throw( db_action_failed );
  string GetValue( string key )
    throw( db_action_failed );
  void AddNodes( string key, string value="" ) throw( db_action_failed );
  void DeleteNodes( string key ) throw( db_action_failed );
  void RenameNodes( string key, string name ) throw( db_action_failed );

private:
  static SCDB *scdb_handle;
  bool modified;
};

#endif
