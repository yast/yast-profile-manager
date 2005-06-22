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
 * converts c++ vector of resource_group_t structures to ycp list of maps
 */
YCPList SCPMAgent::tolist_groups(vector<resource_group_t> l) 
{
    YCPList list;
    unsigned int i;
    
    for ( i=0 ; i<l.size(); i++ ) {
	    list->add(YCPMap(tomap_rg(l[i])));
    }
    return list;
}

/**
 * converts YCP list of maps to c++ vector of resource_group_t structures
 */
vector<resource_group_t> SCPMAgent::fromlist_groups(YCPList list) 
{
    vector<resource_group_t> l;
    int i;
    
    for ( i=0 ; i<list->size(); i++ ) {
        l.push_back ( frommap_rg(list->value(i)->asMap()));
    }
    return l;
}

/**
 * converts c++ resource_info_t to ycp map
 */
YCPMap SCPMAgent::tomap_ri(resource_info_t ri) {
    YCPMap map;

    map->add(YCPString("resource_name"), YCPString(ri.resource_name));
    map->add(YCPString("resource_type"), YCPString(ri.resource_type));
    map->add(YCPString("is_new"), YCPBoolean(ri.is_new));
    map->add(YCPString("is_deleted"), YCPBoolean(ri.is_deleted));
    map->add(YCPString("save"), YCPBoolean(ri.save));

    if (use_rg) {
	string save_mode = "normal";
	switch (ri.save_mode)
	{
	    case normal:
		save_mode = "normal";
		break;
	    case save_all:
		save_mode = "save_all";
		break;
	    case patch_all:
		save_mode = "patch_all";
		break;
	}	
    	map->add(YCPString("save_mode"), YCPString(save_mode));
    	map->add(YCPString("groups"), YCPList(tolist_groups(ri.groups)));
    }

    return map;
}

/**
 * converts ycp map to c++ resource_info_t
 */
resource_info_t SCPMAgent::frommap_ri(YCPMap map) {
    resource_info_t ri;
    
    ri.resource_name = map->value(YCPString("resource_name"))
        ->asString()->value();
    ri.resource_type = map->value(YCPString("resource_type"))
        ->asString()->value();
    ri.is_new = map->value(YCPString("is_new"))
        ->asBoolean()->value();
    ri.is_deleted = map->value(YCPString("is_deleted"))
        ->asBoolean()->value();
    ri.save = map->value(YCPString("save")) ->asBoolean()->value();
	
    if (use_rg) {
	save_mode_t save_mode = normal;
	string save_string = 
		map->value(YCPString("save_mode"))->asString()->value();
	if (save_string	== "save_all")
	    save_mode = save_all;
	if (save_string == "patch_all")
	    save_mode = patch_all;

	ri.save_mode = save_mode;
	ri.groups = fromlist_groups (map->value(YCPString("groups"))->asList());
    }

    return ri;
}

/**
 * converts c++ vector of resource_info_t structures to ycp list of maps
 */
YCPList SCPMAgent::tolist_modified_resources (vector<resource_info_t> l) 
{
    YCPList list;
    unsigned int i;
    
    for ( i=0 ; i<l.size(); i++ ) {
	    list->add(YCPMap(tomap_ri(l[i])));
    }
    return list;
}

/**
 * converts YCP list of maps to c++ vector of resource_info_t structures
 */
vector<resource_info_t> SCPMAgent::fromlist_modified_resources(YCPList list) 
{
    vector<resource_info_t> l;
    int i;
    
    for ( i=0 ; i<list->size(); i++ ) {
        l.push_back ( frommap_ri(list->value(i)->asMap()));
    }
    return l;
}

/**
 * converts switch_info_t structure to YCP map
 */
YCPMap SCPMAgent::tomap_sw(switch_info_t sw) {
    YCPMap map;

    map->add(YCPString("profile_modified"), YCPBoolean(sw.profile_modified));
    map->add(YCPString("profile_name"), YCPString(sw.profile_name));
    map->add(YCPString("modified_resources"), 
            YCPList(tolist_modified_resources(sw.modified_resources)));

    return map;
}

/**
 * converts YCP map to switch_info_t structure
 */
switch_info_t SCPMAgent::frommap_sw(YCPMap map) {
    switch_info_t sw;

    sw.profile_modified = map->value(YCPString("profile_modified"))
        ->asBoolean()->value();
    sw.profile_name = map->value(YCPString("profile_name"))
        ->asString()->value();
    sw.modified_resources = fromlist_modified_resources (
        map->value(YCPString("modified_resources"))->asList());

    return sw;
}


/**
 * Constructor
 */
SCPMAgent::SCPMAgent() : SCRAgent()
{
    options = 0;
    initialized = false;
    use_rg = true;
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
	else if (PC(0) == "profiles") {

	   vector<string> l;
	   if ( !scpm->List( l ) ) {
		y2error( scpm_error );
	    }
	    else
		ret = YCPList(tolist(l));

	}
	else if (PC(0) == "resources") {
     
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
    // traditional branch with two elements
    else if (path->length() == 2) {
	
	if (PC(0) == "profiles") {
	    if (PC(1) == "current") {

		if (!scpm->Active(profile))
		    y2error ( scpm_error );
		else
		    ret = YCPString(profile);
	    }
	    else if ((PC(1) == "description") || (PC(1) == "prestart") ||
		     (PC(1) == "poststart") || (PC(1) == "prestop") ||
		     (PC(1) == "poststop")) {
		string result;
        
		if (!arg.isNull ())
		   profile = arg->asString()->value();
		else {
		    if (!scpm->Active(profile))
			y2error ( scpm_error );
		}
        
		if (!scpm->Get(PC(1), result, profile))
		    y2error (scpm_error);
		else
		    ret = YCPString(result);
	    }
	    else {
		y2error ("Unknown path in Read(): %s",
		       path->toString().c_str());
	    }
	}
	else if ((PC(0) == "resources") && (PC(1) == "current")) {
	    string set;

	    if (!scpm->GetResourceSet(set))
		y2error ( scpm_error );
	    else
		ret = YCPString(set);
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
    if (PC(0) == "profiles") {
	if ((PC(1) == "description") || (PC(1) == "prestart") ||
          (PC(1) == "poststart") || (PC(1) == "prestop") ||
          (PC(1) == "poststop")) {
        
        if ((value.isNull ()) || (!value->isString())) {
	    scpm_error = "Wrong parameter.";
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
	    scpm_error = "Wrong parameter.";
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
    /*
    else if (path->length() == 4) {
    }
    */

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
    
   	if (PC(0) == "switch") {
        
	    if ((value.isNull ()) || (!value->isMap())) {
		scpm_error = "Wrong parameter.";
		y2error ( scpm_error );
		ret = YCPVoid();
	    }
	    else {
		switch_info = frommap_sw(value->asMap());
		pthread_create( &pt, NULL, (void*(*)(void*))&call_switch, this);
		ret = YCPBoolean (true);
	    }
	}

   	if (PC(0) == "save") {
        
	    if ((value.isNull ()) || (!value->isMap())) {
		scpm_error = "Wrong parameter.";
		y2error ( scpm_error );
		ret = YCPVoid();
	    }
	    else {
		switch_info = frommap_sw(value->asMap());
		pthread_create( &pt, NULL, (void*(*)(void*))&call_save, this );
		ret = YCPBoolean (true);
	    }
	}

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

   	if ((PC(0) == "switch") && (PC(1) == "prepare")) {
        
	    if ((value.isNull ()) || (!value->isString())) {
		scpm_error = "Wrong parameter.";
		y2error ( scpm_error );
		ret = YCPVoid();
	    }
	    else {
		profile = value->asString()->value();
		pthread_create( &pt, NULL, (void*(*)(void*))&call_prepare,this);
		ret = YCPBoolean (true);
	    }
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
     
	if (PC(0) == "profiles") {
	    if (PC(1) == "add") {
        
		if ((value.isNull ()) || (arg.isNull ())){
		    scpm_error = "Wrong parameter.";
		    y2error ( scpm_error );
		}
		else {
		    profile = value->asString()->value();
		    auto_switch = arg->asBoolean()->value();
		    pthread_create( &pt, NULL, (void*(*)(void*))&call_add,this);
		    ret = YCPBoolean (true);
		}
	    }
      
	    if (PC(1) == "delete") {
        
		if (value.isNull ()) {
		    scpm_error = "Wrong parameter.";
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
		    scpm_error = "Wrong parameter.";
		    y2error ( scpm_error );
		}
		else {
		    profile = value->asString()->value(); 
		    dest_profile = arg->asString()->value(); 
		    pthread_create(&pt, NULL, (void*(*)(void*))&call_copy,this);
		    ret = YCPBoolean (true);
		}
	    }

	    if (PC(1) == "rename") { 
		string old_profile, new_profile;
        
		if ((value.isNull ()) || (arg.isNull ())) {
		    scpm_error = "Wrong parameter.";
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
		    scpm_error = "Wrong parameter.";
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

/**
 * Call PrepareSwitch in the new thread
 */
void *SCPMAgent::call_prepare( SCPMAgent *ag )
{
  static int retval;
  YCPValue ret = YCPBoolean( true );

  if ( !ag->scpm->PrepareSwitch( ag->profile, ag->switch_info ) ) {
    y2error ( scpm_error );
    ret = YCPVoid();
  }
  else
    ret = YCPMap(ag->tomap_sw(ag->switch_info));

  const char *tmpf = ag->tmpfile.c_str();
  ofstream tmp;
  
  tmp.open (tmpf, ios::out | ios::trunc);
  tmp.write(ret->toString().c_str(), strlen(ret->toString().c_str()));
  tmp.close();
  y2debug("switch_info: %s", ret->toString().c_str());

  retval=0;
  pthread_exit((void*)&retval);
}  

void *SCPMAgent::call_switch( SCPMAgent *ag )
{
  static int retval;

  if ( !ag->scpm->Switch( ag->switch_info ) ) {
    y2error( scpm_error );
    retval=1;
  }
  else 
    retval=0;

  pthread_exit((void*)&retval);
}

void *SCPMAgent::call_save( SCPMAgent *ag )
{
  static int retval;

  if ( !ag->scpm->Save( ag->switch_info ) ) {
    y2error( scpm_error );
    retval=1;
  }
  else 
    retval=0;
  
  pthread_exit((void*)&retval);
}

void *SCPMAgent::call_add( SCPMAgent *ag )
{
  static int retval;

  if (!ag->scpm->Add(ag->profile, ag->auto_switch)) {
      y2error ( scpm_error );
      retval = 1;
  }
  else
      retval = 0;

  pthread_exit((void*)&retval);
}

void *SCPMAgent::call_copy( SCPMAgent *ag )
{
  static int retval;

  if (!ag->scpm->Copy(ag->profile, ag->dest_profile)) {
      y2error ( scpm_error );
      retval = 1;
  }
  else
      retval = 0;
  
  pthread_exit((void*)&retval);
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
