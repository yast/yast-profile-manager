/* SCPMAgent.cc
 *
 * An agent for reading the SCPM configuration file.
 *
 * Authors: Jiri Suchomel <jsuchome@suse.cz>
 *
 * $Id$
 */

#include "SCPMAgent.h"
//#include <pthread.h>

#define PC(n)       (path->component_str(n))

YCPList tolist(vector<string> l) 
{
    YCPList list;
    unsigned int i;
    
    for ( i=0 ; i<l.size(); i++ )
	    list->add(YCPString (l[i]));
    return list;
}


YCPMap tomap_ri(resource_info_t ri) {
    YCPMap map;

    map->add(YCPString("resource_name"), YCPString(ri.resource_name));
    map->add(YCPString("resource_type"), YCPString(ri.resource_type));
    map->add(YCPString("is_new"), YCPBoolean(ri.is_new));
    map->add(YCPString("is_deleted"), YCPBoolean(ri.is_deleted));
    map->add(YCPString("save"), YCPBoolean(ri.save));

    return map;
}

resource_info_t frommap_ri(YCPMap map) {
    resource_info_t ri;
    
    ri.resource_name = map->value(YCPString("resource_name"))
        ->asString()->value();
    ri.resource_type = map->value(YCPString("resource_type"))
        ->asString()->value();
    ri.is_new = map->value(YCPString("is_new"))
        ->asBoolean()->value();
    ri.is_deleted = map->value(YCPString("is_deleted"))
        ->asBoolean()->value();
    ri.save = map->value(YCPString("save"))
        ->asBoolean()->value();

    return ri;
}

YCPList tolist_sw(vector<resource_info_t> l) 
{
    YCPList list;
    unsigned int i;
    
    for ( i=0 ; i<l.size(); i++ ) {
	    list->add(YCPMap(tomap_ri(l[i])));
    }
    return list;
}

vector<resource_info_t> fromlist_sw(YCPList list) 
{
    vector<resource_info_t> l;
    int i;
    
    for ( i=0 ; i<list->size(); i++ ) {
        l.push_back ( frommap_ri(list->value(i)->asMap()));
    }
    return l;
}

YCPMap tomap_sw(switch_info_t sw) {
    YCPMap map;

    map->add(YCPString("profile_modified"), YCPBoolean(sw.profile_modified));
    map->add(YCPString("profile_name"), YCPString(sw.profile_name));
    map->add(YCPString("modified_resources"), 
            YCPList(tolist_sw(sw.modified_resources)));

    return map;
}

switch_info_t frommap_sw(YCPMap map) {
    switch_info_t sw;

    sw.profile_modified = map->value(YCPString("profile_modified"))
        ->asBoolean()->value();
    sw.profile_name = map->value(YCPString("profile_name"))
        ->asString()->value();
    sw.modified_resources = fromlist_sw (
        map->value(YCPString("modified_resources"))->asList());

    return sw;
}


/**
 * Constructor
 */
SCPMAgent::SCPMAgent() : SCRAgent()
{
	options = 0;
	scpm = NULL;
}

/**
 * Destructor
 */
SCPMAgent::~SCPMAgent()
{
}

/**
 * Dir
 */
YCPValue SCPMAgent::Dir(const YCPPath& path)
{
    y2error("Wrong path '%s' in Dir().", path->toString().c_str());
    return YCPVoid();
}

/**
 * Read
 */
YCPValue SCPMAgent::Read(const YCPPath &path, const YCPValue& arg)
{
    y2debug("Path in Read(): %s", path->toString().c_str());
    YCPValue ret = YCPVoid(); 

    if (path->length() == 0) {
    	ret = YCPString("0");
    }
    
    if (path->length() == 1) {
        
   	if (PC(0) == "profiles") {

    	vector<string> l;
	    if ( !scpm->List( l ) ) {
            y2error( scpm_error);
            ret = YCPList();
        }
        else
            ret = YCPList(tolist(l));

    }
    
   	if (PC(0) == "resources") {
     
        vector<string> predefined, individual;
        YCPList sets;
        
        if (!scpm->ListResourceSets(predefined, individual)) {
            y2error ( scpm_error );
        }
        else {
            sets->add (tolist(predefined));
            sets->add (tolist(individual));
            ret = sets;
        }

    }
  	if (PC(0) == "status") {
        scpm_status_t scpm_status;
            
        if (!scpm->Status(scpm_status)) {
            y2error ( scpm_error );
        }
        else {
            YCPList l;
            l->add(YCPBoolean(scpm_status.enabled));
            l->add(YCPBoolean(scpm_status.initialized));
            ret = l;
        }
    }

    if (PC(0) == "exit_status" ) {
        int *retval;
        ret = YCPBoolean(false);
        if ( pthread_join( pt, (void**)&retval ) == 0 ) 
            if ( *retval == 0 )
                ret = YCPBoolean(true);
    }


    }
    
    if (path->length() == 2) {
        
    /* not necessary any more:
   	if ((PC(0) == "status") && (PC(1) == "enabled")) {
        scpm_status_t scpm_status;
            
        if (!scpm->Status(scpm_status)) {
            y2error ( scpm_error );
        }
        else 
            ret = YCPBoolean(scpm_status.enabled);
    }*/

	if (PC(0) == "profiles") {
      if (PC(1) == "current") {

        if (!scpm->Active(profile)) {
            y2error ( scpm_error );
        }
        else
            ret = YCPString(profile);
      }
      
      if ((PC(1) == "description") || (PC(1) == "prestart") ||
          (PC(1) == "poststart") || (PC(1) == "prestop") ||
          (PC(1) == "poststop")) {
        string result;
        
        if (!arg.isNull ()) profile = arg->asString()->value();
        else {
            if (!scpm->Active(profile)) {
                ret = YCPString(""); // I do not check void value in module
                y2error ( scpm_error );
            }
        }
        
        if (!scpm->Get(PC(1), result, profile)) {
            ret = YCPString("");
            y2error (scpm_error);
        }
        else
            ret = YCPString(result);
      }
          
    
    }
     
   	if ((PC(0) == "resources") && (PC(1) == "current")) {
        string set;

        if (!scpm->GetResourceSet(set)) {
            y2error ( scpm_error );
        }
        else
            ret = YCPString(set);
    }
    
    } 
    return ret;
}


/**
 * Write(.scpm, nil) mast be run finally
 */
YCPValue SCPMAgent::Write(const YCPPath &path, const YCPValue& value,
			     const YCPValue& arg)
{
    y2debug("Path in Write(): %s", path->toString().c_str());
    YCPValue ret = YCPBoolean(false);

    if (path->length() == 0) {
        y2debug("---------- destructing SCPM object");
    //    hash.close();
        output.close();
        if (scpm) delete scpm;
        ret = YCPBoolean(true);
    }

    if (path->length() == 2) {
        
   	if ((PC(0) == "status") && (PC(1) == "enabled")) {

        bool action;

        if (value.isNull ()) {
            y2error ( scpm_error );
            ret = YCPVoid();
        }

        if (!value->isBoolean ()) {
            y2error (scpm_error);
        }
        else {
            action = value->asBoolean()->value();
            bool force = false; 
            if (action) {
                if (!scpm->Enable(force)) 
                    y2error ( scpm_error );
                else
                    ret = YCPBoolean(1);
            }
            else {
                if (!scpm->Disable()) 
                    y2error ( scpm_error );
                else
                    ret = YCPBoolean(1);
            
            }
        }

    } 
	if (PC(0) == "profiles") {
      if ((PC(1) == "description") || (PC(1) == "prestart") ||
          (PC(1) == "poststart") || (PC(1) == "prestop") ||
          (PC(1) == "poststop")) {
        
        if ((value.isNull ()) || (!value->isString())) {
            y2error ( scpm_error );
        }
        else {
            if (!arg.isNull ()) 
                profile = arg->asString()->value();
            else {
                if (!scpm->Active(profile)) 
                    y2error ( scpm_error );
            }
            
            if (!scpm->Set(PC(1), value->asString()->value(), profile)) 
                y2error ( scpm_error );
            else
                ret = YCPBoolean(1);
        }
      }
    }
   	if ((PC(0) == "resources") && (PC(1) == "current")) {
        
        if ((value.isNull ()) || (!value->isString())) {
            y2error ( scpm_error );
        }
        else {
            if (!scpm->SetResourceSet(value->asString()->value())) 
                y2error ( scpm_error );
            else
                ret = YCPBoolean(1);
        }
    }

    }

    return ret;

}
 
/**
 * Execute(.scpm) must be run first to initialize
 */
YCPValue SCPMAgent::Execute(const YCPPath &path, const YCPValue& value,
			     const YCPValue& arg)
{
    y2debug("Path in Execute(): %s", path->toString().c_str());
    YCPValue ret = YCPBoolean(false);
    
    if (path->length() == 0) {
        string tmpdir = value->asString()->value();
        
        string outfile = tmpdir + "/scpm.progress";
//        string 
        hashfile = tmpdir + "/scpm.hash";
        changesfile = tmpdir + "/scpm.changes";
        tmpfile = tmpdir + "/scpm.tmp";

        const char *outf = outfile.c_str();
//        const char *hashf = hashfile.c_str();
    
        y2debug("----------- agent-scpm initialization");
//        y2debug("----------- output to: %s, %s", outf, hashf);
    	output.open (outf);
//    	hash.open (hashf);
        scpm = new SCPM (options, output);//, hash);
        if (scpm)
            ret = YCPBoolean(true);
    }
    
    if (path->length() == 1) {
    
   	if (PC(0) == "switch") {
        
        if ((value.isNull ()) || (!value->isMap())) {
            y2error ( scpm_error );
            ret = YCPVoid();
        }
        else {
            switch_info = frommap_sw(value->asMap());
            pthread_create( &pt, NULL, (void*(*)(void*))&call_switch, this );
        }
    }
    
    }
    
    if (path->length() == 2) {
        
   	if ((PC(0) == "switch") && (PC(1) == "prepare")) {
        
        if ((value.isNull ()) || (!value->isString())) {
            y2error ( scpm_error );
            ret = YCPVoid();
        }
        else {
            profile = value->asString()->value();
            pthread_create( &pt, NULL, (void*(*)(void*))&call_prepare, this );
        }
    }

   	if ((PC(0) == "enable") && (PC(1) == "first")) {
        
        pthread_create( &pt, NULL, (void*(*)(void*))&call_enable, this );
    }
		
   	if (PC(0) == "resources") {
      if (PC(1) == "rebuild") {
        
		if (!scpm->RebuildDB()) {
            y2error ( scpm_error );
        }
        else
            ret = YCPBoolean(true);
      }
      if (PC(1) == "delete") {
        string set;
        
        if (value.isNull ()) {
            y2error ( scpm_error );
        }
        else {
            set = value->asString()->value();
/*        
            if (!scpm->DeleteResourceSet(set)) 
                y2error ( scpm_error );
            else*/
                ret = YCPBoolean(1);
        }
      }
      if (PC(1) == "copy") {
        string set, newset;

        if ((value.isNull ()) || arg.isNull ()){
            y2error ( scpm_error );
        }
        else {
            set = value->asString()->value();
            newset = arg->asString()->value();
       /* 
            if (!scpm->CopyResourceSet(set, newset)) 
                y2error ( scpm_error );
            else*/
                ret = YCPBoolean(1);
        }
      }
      

    }
     
   	if (PC(0) == "profiles") {
      if (PC(1) == "add") {
        
        if ((value.isNull ()) || (arg.isNull ())){
            y2error ( scpm_error );
        }
        else {
            profile = value->asString()->value();
            auto_switch = arg->asBoolean()->value();
            pthread_create( &pt, NULL, (void*(*)(void*))&call_add, this );
        }
      }
      
      if (PC(1) == "delete") {
        
        if (value.isNull ()) {
            y2error ( scpm_error );
        }
        else {
            profile = value->asString()->value();
        
            if (!scpm->Delete(profile)) 
                y2error ( scpm_error );
            else
                ret = YCPBoolean(1);
        }
      }

      if (PC(1) == "copy") { 
        
        if ((value.isNull ()) || (arg.isNull ())) {
            y2error ( scpm_error );
        }
        else {
            profile = value->asString()->value(); 
            dest_profile = arg->asString()->value(); 
            pthread_create( &pt, NULL, (void*(*)(void*))&call_copy, this );
        }
      }

      if (PC(1) == "rename") { 
        string old_profile, new_profile;
        
        if ((value.isNull ()) || (arg.isNull ())) {
            y2error ( scpm_error );
        }
        else {
            old_profile = value->asString()->value(); 
            new_profile = arg->asString()->value(); 
        
            if (!scpm->Rename(old_profile, new_profile)) 
                y2error ( scpm_error );
            else
                ret = YCPBoolean(1);
        }
      }

      if (PC(1) == "changes") { 
        string name, type;
        ofstream changes;
    
        if ((value.isNull ()) || (arg.isNull ())) {
            y2error ( scpm_error );
        }
        else {
            const char *chgf = changesfile.c_str();
            type = value->asString()->value(); 
            name = arg->asString()->value(); 
            changes.open (chgf);
            y2debug("----------- changes to: %s", chgf);
        
            if (!scpm->ShowChanges(changes, type, name)) 
                y2error ( scpm_error );
            else
                ret = YCPBoolean(1);
            changes.close();
        }
      }
      
    }
    }
     
    return ret;
}

/**
 * otherCommand
 */
YCPValue SCPMAgent::otherCommand(const YCPTerm& term)
{
    string sym = term->symbol()->symbol();

    if (sym == "SCPMAgent") {
        
        return YCPVoid();
    }

    return YCPNull();
}

void *SCPMAgent::call_prepare( SCPMAgent *ag )
{
  static int retval;
  YCPValue ret = YCPBoolean( true );

  const char *hashf = ag->hashfile.c_str();
  ofstream hash;

  hash.open (hashf, ios::out | ios::trunc);

  if ( !ag->scpm->PrepareSwitch( ag->profile, ag->switch_info ) ) {
    y2error ( scpm_error );
    ret = YCPVoid();
  }
  else
    ret = YCPMap(tomap_sw(ag->switch_info));

  const char *tmpf = ag->tmpfile.c_str();
  ofstream tmp;
  
  tmp.open (tmpf, ios::out | ios::trunc);
  tmp.write(ret->toString().c_str(), strlen(ret->toString().c_str()));
  tmp.close();
  y2debug("switch_info: %s", ret->toString().c_str());

  int i;
  for (i=0; i<10; i++)
    hash.write("##########", 10);
  hash.close();
  
  retval=0;
  pthread_exit((void*)&retval);
}  

void *SCPMAgent::call_switch( SCPMAgent *ag )
{
  static int retval;

  const char *hashf = ag->hashfile.c_str();
  ofstream hash;

  hash.open (hashf, ios::out | ios::trunc);

  if ( !ag->scpm->Switch( ag->switch_info ) ) {
    y2error( scpm_error );
    retval=1;
  }
  else 
    retval=0;

  int i;
  for (i=0; i<10; i++)
    hash.write("##########", 10);
  hash.close();

  pthread_exit((void*)&retval);
}

void *SCPMAgent::call_add( SCPMAgent *ag )
{
  static int retval;

  const char *hashf = ag->hashfile.c_str();
  ofstream hash;

  hash.open (hashf, ios::out | ios::trunc);

  if (!ag->scpm->Add(ag->profile, ag->auto_switch)) {
      y2error ( scpm_error );
      retval = 1;
  }
  else
      retval = 0;

  int i;
  for (i=0; i<10; i++)
    hash.write("##########", 10);
  hash.close();

  pthread_exit((void*)&retval);
}

void *SCPMAgent::call_copy( SCPMAgent *ag )
{
  static int retval;

  const char *hashf = ag->hashfile.c_str();
  ofstream hash;

  hash.open (hashf, ios::out | ios::trunc);

  if (!ag->scpm->Copy(ag->profile, ag->dest_profile)) {
      y2error ( scpm_error );
      retval = 1;
  }
  else
      retval = 0;

  int i;
  for (i=0; i<10; i++)
    hash.write("##########", 10);
  hash.close();

  pthread_exit((void*)&retval);
}

void *SCPMAgent::call_enable( SCPMAgent *ag )
{
  static int retval;

  const char *hashf = ag->hashfile.c_str();
  ofstream hash;

  hash.open (hashf, ios::out | ios::trunc);

  bool force = false;
  if (!ag->scpm->Enable(force)) {
      y2error ( scpm_error );
      retval = 1;
  }
  else
      retval = 0;

  int i;
  for (i=0; i<10; i++)
    hash.write("##########", 10);
  hash.close();

  pthread_exit((void*)&retval);
}

