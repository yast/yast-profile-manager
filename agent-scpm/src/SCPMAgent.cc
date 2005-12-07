/* SCPMAgent.cc
 *
 * An agent for reading the SCPM configuration file.
 *
 * Authors: Jiri Suchomel <jsuchome@suse.cz>
 *
 * $Id$
 */

#include "SCPMAgent.h"

#define PC(n)       (path->component_str(n))

/**
 * convert vector of strings to YCP list
 */
YCPList tolist(vector<string> l) 
{
    YCPList list;
    unsigned int i;
    
    for ( i=0 ; i<l.size(); i++ )
	    list->add(YCPString (l[i]));
    return list;
}

/**
 * converts c++ structure of type resource_entry_t to ycp map
 */
YCPMap SCPMAgent::tomap_re(resource_entry_t rentry) {
    YCPMap map;

    map->add ( YCPString("name"), YCPString (rentry.name));
    map->add ( YCPString("type"), YCPString (rentry.type));
    map->add ( YCPString("user_defined"), YCPBoolean(rentry.user_defined));
    map->add ( YCPString("active"), YCPBoolean(rentry.active));
	return map;
}

/**
 * converts ycp map to c++ structure of type resource_entry_t 
 */
resource_entry_t SCPMAgent::frommap_re(YCPMap map) {

    resource_entry_t re;
    
    re.name = map->value(YCPString("name"))->asString()->value();
    re.type = map->value(YCPString("type"))->asString()->value();
    re.active = map->value(YCPString("active"))->asBoolean()->value();
    re.user_defined = map->value(YCPString("user_defined"))
		->asBoolean()->value();
	return re;	
}

/**
 * converts ycp list of maps to c++ vector of resource_entry_t structures
 */
vector<resource_entry_t> SCPMAgent::fromlist_re(YCPList list) {
    vector<resource_entry_t> l;
    int i;
    
    for ( i=0 ; i<list->size(); i++ ) {
        l.push_back ( frommap_re(list->value(i)->asMap()));
    }
    return l;
}

/**
 * converts c++ structure of type resource_group_t to ycp map
 */
YCPMap SCPMAgent::tomap_rg(resource_group_t rgroup) {
    YCPMap map;

    map->add ( YCPString("name"), YCPString (rgroup.name));
    map->add ( YCPString("description"), YCPString (rgroup.description));
    map->add ( YCPString("user_defined"), YCPBoolean(rgroup.user_defined));
    map->add ( YCPString("user_modified"), YCPBoolean(rgroup.user_modified));
    map->add ( YCPString("active"), YCPBoolean(rgroup.active));
	return map;
}

/**
 * converts ycp map to c++ structure of type resource_group_t 
 */
resource_group_t SCPMAgent::frommap_rg(YCPMap map) {

    resource_group_t rg;
    
    rg.name	= map->value(YCPString("name"))->asString()->value();
    rg.description = map->value(YCPString("description"))->asString()->value();
    rg.active	= map->value(YCPString("active"))->asBoolean()->value();
    rg.user_defined = map->value(YCPString("user_defined"))
	    ->asBoolean()->value();
    rg.user_modified = map->value(YCPString("user_modified"))
	    ->asBoolean()->value();
    return rg;	
}

/**
 * Constructor
 */
SCPMAgent::SCPMAgent() : SCRAgent()
{
    options = 0;
    initialized = false;
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
YCPList SCPMAgent::Dir(const YCPPath& path) 
{   
    y2error("Wrong path '%s' in Dir().", path->toString().c_str());
    return YCPNull();
}


/**
 * Read
 */
YCPValue SCPMAgent::Read(const YCPPath &path, const YCPValue& arg, const YCPValue& opt) {

    y2debug("Path in Read(): %s", path->toString().c_str());
    YCPValue ret = YCPVoid(); 

    if (!initialized)
    {
	scpm_error = "SCPM not initialized yet!";
        y2error (scpm_error);
        return ret;
    }

    if (path->length() == 0) {
    	ret = YCPString("0");
    }
    // the new branch for reading resource groups
    else if (PC(0) == "rg") {
	
	// (scpm.rg): get list of resource groups
	// returning map indexed by name
	if (path->length() == 1) {
    
	    vector <resource_group_t> groups;
    
	    if (!scpm->ListResourceGroups(groups)) {
            	y2error ( scpm_error );
	    }
	    else {
		// to get a list:
		// YCPList resource_groups = tolist_groups(groups));
        	YCPMap resource_groups;
   		for ( unsigned int i=0 ; i<groups.size(); i++ ) {
		    resource_groups->add (
			YCPString(groups[i].name), tomap_rg(groups[i]));
        	}
		ret = resource_groups;
	    }
	}
	else if (path->length() == 2) {

	    // (.scpm.rg.group, name): get info about one resource group
	    // returns list of resources
	    if (PC(1) == "group") {

		if (arg.isNull()) {		
		    scpm_error = "Wrong parameter.";
		    y2error ( scpm_error );
		}
		else {
		    string groupname = arg->asString()->value();
		    string desc;
		    vector<resource_entry_t> group;
				
		    if (!scpm->GetResourceGroup (groupname, group, desc,false))
		    {
           		y2error ( scpm_error );
		    }
		    else {
            		YCPList resource_group;
			for ( unsigned int i=0; i<group.size(); i++ ) {
			    resource_group->add (tomap_re (group[i]));
			}
			ret = resource_group;
		    }
		}
	    }
	    // (.scpm.rg.group_map, name): get more info about one rg
	    // map of group
	    else if (PC(1) == "group_map" || PC(1) == "group_default") {

		if (arg.isNull()) {		
		    scpm_error = "Wrong parameter.";
		    y2error ( scpm_error );
		}
		else {
		    string groupname = arg->asString()->value();
		    string desc;
		    vector<resource_entry_t> group;
		    bool def	= false;
		    if (PC(1) == "group_default")
			def	= true;
				
		    if (!scpm->GetResourceGroup (groupname, group, desc, def)) {
           		y2error ( scpm_error );
		    }
		    else {
			YCPMap resource_group;
            		YCPList resources;
			for ( unsigned int i=0; i<group.size(); i++ ) {
			    resources->add (tomap_re (group[i]));
			}
			resource_group->add (
			    YCPString ("description"), YCPString (desc));
			resource_group->add (
			    YCPString ("resources"), resources);
			ret = resource_group;
		    }
		}
	    }
	}
	else {
	    y2error ("Unknown path in Read(): %s", path->toString().c_str());
	}
    }
    // traditional branch with one element
    else if (path->length() == 1) {
        
	if (PC(0) == "error") {
	    // return the last error message
	    ret = YCPString (scpm_error);
	}
	else if (PC(0) == "status") {
	    scpm_status_t scpm_status;
            
	    if (!scpm->Status(scpm_status)) {
		y2error ( scpm_error );
	    }
	    else {
		YCPMap retmap;
		retmap->add (YCPString ("enabled"),
			     YCPBoolean (scpm_status.enabled));
		retmap->add (YCPString ("initialized"),
			     YCPBoolean (scpm_status.initialized));
		retmap->add (YCPString ("scpm_version"),
			     YCPString (scpm_status.scpm_version));
		retmap->add (YCPString ("needs_reinit"),
			     YCPBoolean (scpm_status.needs_reinit));
		retmap->add (YCPString ("needs_recover"),
			     YCPBoolean (scpm_status.needs_recover));
		retmap->add (YCPString ("active_profile"),
			     YCPString (scpm_status.active_profile));
		ret = retmap;
	    }
	}
	else if (PC(0) == "exit_status" ) {
	    int *retval;
	    ret = YCPBoolean(false);
	    if ( pthread_join( pt, (void**)&retval ) == 0 ) 
		if ( *retval == 0 )
		    ret = YCPBoolean(true);
	}
	else {
	    y2error ("Unknown path in Read(): %s", path->toString().c_str());
	}
    }
    else {
       y2error ("Unknown path in Read(): %s", path->toString().c_str());
    }
    return ret;
}


/**
 * Write(.scpm, nil) mast be run finally
 */
YCPBoolean SCPMAgent::Write(const YCPPath &path, const YCPValue& value,
			     const YCPValue& arg)
{
    y2debug("Path in Write(): %s", path->toString().c_str());
    YCPBoolean ret = YCPBoolean(false);

    if (!initialized)
    {
        y2error ("SCPM not initialized yet!");
        return ret;
    }

    if (path->length() == 0) {
        y2debug("---------- destructing SCPM object");
        hash.close();
        output.close();
        if (scpm)
        {
           delete scpm;
           ret = YCPBoolean(true);
        }
    }
    else if (path->length() == 2) {
        
      if ((PC(0) == "status") && (PC(1) == "enabled")) {

        bool action;

        if (value.isNull ()) {
	    scpm_error = "Wrong parameter.";
            y2error ( scpm_error );
        }

        if (!value->isBoolean ()) {
	    scpm_error = "Wrong parameter.";
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
    }
    else if (path->length() >= 3) {

	// (scpm.rg.group.delete, groupname): delete resource group
	if ((PC(0) == "rg") && PC(1) == "group" && PC(2) == "delete") {

	    if (!scpm->DeleteResourceGroup (value->asString()->value())) {
		y2error ( scpm_error );
	    }
	    else {
            	ret = YCPBoolean(1);
	    }
	}
	
	// (scpm.rg.group.set.groupname, list, descr): save resource group
	else if ((PC(0) == "rg") && PC(1) == "group" && PC(2) == "set") {

	    if ((value.isNull ()) || (arg.isNull())) {
		scpm_error = "Wrong parameters.";
            	y2error ( scpm_error );
            }
	    else {
		if (!scpm->SetResourceGroup (PC(3),
			    fromlist_re (value->asList()),
			    arg->asString()->value())) {
		    y2error ( scpm_error );
		}
		else {
		    ret = YCPBoolean(1);
		}
	    }
	}		
	// (scpm.rg.group.rename.groupname, new_name): rename resource group
	else if ((PC(0) == "rg") && PC(1) == "group" && PC(2) == "rename") {

	    if (value.isNull ()) {
		scpm_error = "Wrong parameters.";
            	y2error ( scpm_error );
            }
	    else {
		if (!scpm->RenameResourceGroup (PC(3),
			    value->asString()->value())) {
		    y2error ( scpm_error );
		}
		else {
		    ret = YCPBoolean(1);
		}
	    }
	}		
	// (scpm.rg.group.activate, groupname, boolean): (de)activate res. group
	else if ((PC(0) == "rg") && PC(1) == "group" && PC(2) == "activate") {

	    if ((value.isNull ()) || (arg.isNull())) {
		scpm_error = "Wrong parameters.";
            	y2error ( scpm_error );
            }
	    else {
		string name = value->asString()->value();
		if (arg->asBoolean()->value()) {
		    if (!scpm->ActivateResourceGroup (name))
			y2error ( scpm_error );
		    else
			ret = YCPBoolean(1);
		}
		else {
		    if (!scpm->DeactivateResourceGroup (name))
			y2error ( scpm_error );
		    else
			ret = YCPBoolean(1);
		}
	    }	
	}
	else if ((PC(0) == "status")&&(PC(1) == "enabled")&&(PC(2)=="force")) {

            if (!scpm->Enable(true)) 
                y2error ( scpm_error );
            else
                ret = YCPBoolean(1);
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
    y2debug ("Path in Execute(): %s", path->toString().c_str());
    YCPValue ret = YCPBoolean(false);
    
    if (path->length() == 0) {
        string tmpdir = value->asString()->value();
        
        string outfile = tmpdir + "/scpm.progress";
        hashfile = tmpdir + "/scpm.hash";
        changesfile = tmpdir + "/scpm.changes";
        tmpfile = tmpdir + "/scpm.tmp";

        const char *outf = outfile.c_str();
        const char *hashf = hashfile.c_str();
    
        y2debug("----------- agent-scpm initialization");
        y2debug("----------- output to: %s, %s", outf, hashf);
    	output.open (outf);
    	hash.open (hashf);
        options |= scpm_flag_hash; // enable progress bar
        scpm = new SCPM (options, output, hash);
        if (scpm)
        {
            ret = YCPBoolean(true);
            initialized = true;
        }
    }

    if (!initialized)
    {
        y2error ("SCPM not initialized!");
        return ret;
    }
 
    if (path->length() == 1) {
    
   	if (PC(0) == "recover") {
        
	    pthread_create (&pt, NULL, (void*(*)(void*))&call_recover, this);
	    ret = YCPBoolean (true);
	}

   	if (PC(0) == "rollback") {
        
	    pthread_create (&pt, NULL, (void*(*)(void*))&call_rollback, this);
	    ret = YCPBoolean (true);
	}
    }
    else if (path->length() == 2) {
	
	// (.scpm.rg.reset): reset all resource groups
   	if ((PC(0) == "rg") && (PC(1) == "reset")) {

	    if (!scpm->ResetAllGroups ())
		y2error ( scpm_error );
	    else
		ret = YCPBoolean(true);
	}

   	if ((PC(0) == "enable") && (PC(1) == "first")) {
        
	    pthread_create( &pt, NULL, (void*(*)(void*))&call_enable, this );
	    ret = YCPBoolean (true);
	}
		
	if (PC(0) == "resources") {
	    if (PC(1) == "rebuild") {
        
		if (!scpm->RebuildDB())
		    y2error ( scpm_error );
		else
		    ret = YCPBoolean(true);
	    }
	}
    }
    else if (path->length() == 3) {
	
	// (.scpm.rg.group.reset, group_name): reset resource group group_name
   	if ((PC(0) == "rg") && (PC(1) == "group") && (PC(2) == "reset")) {

	    if ((value.isNull ()) || (!value->isString())) {
		scpm_error = "Wrong parameters.";
            	y2error ( scpm_error );
            }
	    else
	    {
		if (!scpm->ResetResourceGroup (value->asString()->value()))
		    y2error ( scpm_error );
		else
		    ret = YCPBoolean(true);
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
    string sym = term->name();

    if (sym == "SCPMAgent") {
        
        return YCPVoid();
    }

    return YCPNull();
}

void *SCPMAgent::call_enable( SCPMAgent *ag )
{
  static int retval;
  
  bool force = false;
  if (!ag->scpm->Enable(force)) {
      y2error ( scpm_error );
      retval = 1;
  }
  else
      retval = 0;

  pthread_exit((void*)&retval);
}

void *SCPMAgent::call_recover (SCPMAgent *ag)
{
  static int retval;
  
  if (!ag->scpm->Recover ()) {
      y2error ( scpm_error );
      retval = 1;
  }
  else
      retval = 0;

  pthread_exit((void*)&retval);
}

void *SCPMAgent::call_rollback (SCPMAgent *ag)
{
  static int retval;
  
  if (!ag->scpm->Recover (true)) {
      y2error ( scpm_error );
      retval = 1;
  }
  else
      retval = 0;

  pthread_exit((void*)&retval);
}
