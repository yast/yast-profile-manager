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
#define VAL2STR(v)  ((v)->asString()->value())
#define VAL2CSTR(v) ((v)->asString()->value_cstr())

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
    map->add(YCPString("id_new"), YCPBoolean(ri.is_new));
    map->add(YCPString("id_deleted"), YCPBoolean(ri.is_deleted));
    map->add(YCPString("save"), YCPBoolean(ri.save));

    return map;
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

YCPMap tomap_sw(switch_info_t sw) {
    YCPMap map;

    map->add(YCPString("profile_modified"), YCPBoolean(sw.profile_modified));
    map->add(YCPString("profile_name"), YCPString(sw.profile_name));
    map->add(YCPString("modified_resources"), 
            YCPList(tolist_sw(sw.modified_resources)));

    return map;
}


/**
 * Constructor
 */
SCPMAgent::SCPMAgent() : SCRAgent()
{
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
    y2error("Path in Read(): %s", path->toString().c_str());
    
    options = 0;
    scpm = NULL;
    YCPValue ret = YCPVoid(); 

    scpm = new SCPM (options);
    
    if (path->length() == 0) {
    	ret = YCPString("0");
    }
    
    if (path->length() == 1) {
        
   	if (PC(0) == "profiles") {

    	vector<string> l;
	    if ( !scpm->List( l ) ) {
            y2error ("error");
            ret = YCPList();
        }
             //die( scpm_error, ERR_SUBSYS );
        else
            ret = YCPList(tolist(l));

    }
    
   	if (PC(0) == "resources") {
     
        vector<string> predefined, individual;
        YCPList sets;
        
        if (!scpm->ListResourceSets(predefined, individual)) {
            y2error ("error");
            sets->add(YCPList());
            sets->add(YCPList());
            ret = sets;
        }
        else {
            sets->add (tolist(predefined));
            sets->add (tolist(individual));
            ret = sets;
        }

    }
	
    if (PC(0) == "progress")
        ret = YCPString("progress");
    
	if (PC(0) == "hash")
        ret = YCPString("hash");

    }
    
    if (path->length() == 2) {
        
   	if ((PC(0) == "status") && (PC(1) == "enabled")) {
        scpm_status_t scpm_status;

        if (!scpm->Status(scpm_status)) 
            y2error ("error");
        else
            ret = YCPBoolean(scpm_status.enabled);
    }

	if (PC(0) == "profiles") {
      if (PC(1) == "current") {
        string profile;

        if (!scpm->Active(profile)) 
            y2error ("error");
        else
            ret = YCPString(profile);
      }
      
      if ((PC(1) == "description") || (PC(1) == "prestart") ||
          (PC(1) == "poststart") || (PC(1) == "prestop") ||
          (PC(1) == "poststop")) {
        string profile, result;
        
        if (!arg.isNull ()) profile = arg->asString()->value();
        else {
            if (!scpm->Active(profile)) 
                y2error ("error(a)");
        }
        
        if (!scpm->Get(PC(1), result, profile)) 
            y2error ("error(get)");
        else
            ret = YCPString(result);
      }
          
    
    }
     
   	if ((PC(0) == "resources") && (PC(1) == "current")) {
        string set;
        if (!scpm->GetResourceSet(set)) 
            y2error ("error");
        else
            ret = YCPString(set);
    }
    
    } 
    
    if (scpm) delete scpm;

    return ret;
}


YCPValue SCPMAgent::Write(const YCPPath &path, const YCPValue& value,
			     const YCPValue& arg)
{
    y2error("Path in Write(): %s", path->toString().c_str());

    options = 0;
    scpm = NULL;
    YCPBoolean ret = false;

    scpm = new SCPM (options);

    if (path->length() == 2) {
        
   	if ((PC(0) == "status") && (PC(1) == "enabled")) {

        bool action;

/*        if (!value.isNull ()) {
            y2error ("error1");
            ret = YCPVoid();
        }*/

        if (!value->isBoolean ()) {
            y2error ("error2");
        }
        else {
            action = value->asBoolean()->value();
            // new
        
            if (action) {
                if (!scpm->Enable()) 
                    y2error ("error(e)");
                else
                    ret = YCPBoolean(1);
            }
            else {
                if (!scpm->Disable()) 
                    y2error ("error(d)");
                else
                    ret = YCPBoolean(1);
            
            }
            // delete
        }

    } 
	if (PC(0) == "profiles") {
      if ((PC(1) == "description") || (PC(1) == "prestart") ||
          (PC(1) == "poststart") || (PC(1) == "prestop") ||
          (PC(1) == "poststop")) {
        string profile;
        
        if ((value.isNull ()) || (!value->isString())) {
            y2error ("error");
        }
        else {
            if (!arg.isNull ()) 
                profile = arg->asString()->value();
            else {
                if (!scpm->Active(profile)) 
                    y2error ("error(active)");
            }
        
            if (!scpm->Set(PC(1), value->asString()->value(), profile)) 
                y2error ("error(set)");
            else
                ret = YCPBoolean(1);
        }
      }
    }
   	if ((PC(0) == "resources") && (PC(1) == "current")) {
        
        if ((value.isNull ()) || (!value->isString())) {
            y2error ("error");
        }
        else {
            if (!scpm->SetResourceSet(value->asString()->value())) 
                y2error ("error");
            else
                ret = YCPBoolean(1);
        }
    }

    }

    if (scpm) delete scpm;

    return ret;

}
 

/**
 * 
 */
YCPValue SCPMAgent::Execute(const YCPPath &path, const YCPValue& value,
			     const YCPValue& arg)
{
    y2error("Path in Execute(): %s", path->toString().c_str());
    
    options = 0;
    scpm = NULL;
    YCPValue ret = YCPBoolean(false);
    ofstream vystup;
    
    vystup.open ("/tmp/vystup.out");
    scpm = new SCPM (options, vystup);

    if (path->length() == 0) {
    }
    
    if (path->length() == 1) {
    
   	if (PC(0) == "switch") {
        switch_info_t switch_info;
        
        if (!scpm->Switch(switch_info)) 
            y2error ("error");
        else {
            ret = YCPBoolean(1);
        }

    }
    }
    
    if (path->length() == 2) {
        
   	if ((PC(0) == "switch") && (PC(1) == "prepare")) {
        string profile;
        switch_info_t switch_info;
        
        if ((value.isNull ()) || (!value->isString())) {
            y2error ("error");
            ret = YCPVoid();
        }
        else {
            profile = value->asString()->value();
            if (!scpm->PrepareSwitch(profile, switch_info)) {
                y2error ("error");
                ret = YCPVoid();
            }
            else
                ret = YCPMap(tomap_sw(switch_info));
        }
    }
    
   	if (PC(0) == "profiles") {
      if (PC(1) == "add") {
        string profile;
        
        if (value.isNull ()) {
            y2error ("error");
        }
        else {
            profile = value->asString()->value();
        
            if (!scpm->Add(profile)) // auto_switch!!!
                y2error ("error");
            else
                ret = YCPBoolean(1);
        }
      }
      
      if (PC(1) == "delete") {
        string profile;
        
        if (value.isNull ()) {
            y2error ("error");
        }
        else {
            profile = value->asString()->value();
        
            if (!scpm->Delete(profile)) 
                y2error ("error");
            else
                ret = YCPBoolean(1);
        }
      }

      if (PC(1) == "copy") { 
        string dest_profile, source_profile;
        
        if ((value.isNull ()) || (arg.isNull ())) {
            y2error ("error");
        }
        else {
            source_profile = value->asString()->value(); 
            dest_profile = arg->asString()->value(); 
        
            if (!scpm->Copy(source_profile, dest_profile)) 
                y2error ("error");
            else
                ret = YCPBoolean(1);
        }
      }

      if (PC(1) == "rename") { 
        string profile, new_profile;
        
        if ((value.isNull ()) || (arg.isNull ())) {
            y2error ("error");
        }
        else {
            profile = value->asString()->value(); 
            new_profile = arg->asString()->value(); 
        
            if (!scpm->Rename(profile, new_profile)) 
                y2error ("error");
            else
                ret = YCPBoolean(1);
        }
      }
      
      
    }
    }
     
    vystup.close();
    if (scpm) delete scpm;

    return ret;
}

/**
 * otherCommand
 */
YCPValue SCPMAgent::otherCommand(const YCPTerm& term)
{
    string sym = term->symbol()->symbol();

    if (sym == "SCPMAgent") {
        
        /* Your initialization */
        
        
        return YCPVoid();
    }

    return YCPNull();
}




/* ModulesAgent.cc
 *
 * An agent for reading the modules.conf configuration file.
 *
 * Author: Michal Svec <msvec@suse.cz>
 *         Daniel Vesely <dan@suse.cz>
 *
 * $Id$
 *
 *


#include "Y2Logger.h"
#include "ModulesAgent.h"
#include "ModulesConf.h"

#define MAGIC_DIRECTIVE "extra"

#define PC(n)       (path->component_str(n))
#define VAL2STR(v)  ((v)->asString()->value())
#define VAL2CSTR(v) ((v)->asString()->value_cstr())

*
  PATH:
        MAP[STRING].MAP[STRING].MAP[STRING].(MAP[STRING].STRING|STRING)

  READ:
        .modules.options.<module>.<option>
                           "options".STRING.MAP
        .modules.options.<module>.<option>.<parameters>
                           "options".STRING.STRING.STRING
        .modules.<directive>.<module>.<argument>
                           STRING.STRING.STRING
        .modules.<directive>.<module>.comment.<comment>
                           STRING.STRING."comment".STRING
        .modules.<directive>.<module>
                           STRING.STRING.LIST
        .modules.<directive>
                           STRING.LIST
        .modules
                           LIST

ModulesAgent::ModulesAgent() : SCRAgent(), modules_conf(NULL) {
}


ModulesAgent::~ModulesAgent() {
    if (modules_conf != NULL)
	delete modules_conf;
}

**
 * Simple template function for converting c++ map into YCP list
 *
template <class T> YCPList map2list(const T &m) {
    YCPList list;
    typename T::const_iterator it = m.begin ();
	
    for (; it != m.end (); ++it)
	list->add(YCPString (it->first));
	
    return list;
}

**
 * Simple template function for converting c++ map into YCP map
 *
template <class T> YCPMap map2ycpmap(const T &m) {
    YCPMap ret_map;
    typename T::const_iterator it = m.begin ();
	
    for (; it != m.end(); ++it)
	ret_map->add (YCPString (it->first), YCPString (it->second));
	
    return ret_map;
}

**
 * Simple function for converting YCP map into c++ map
 *
ModuleEntry::EntryArg ycpmap2map (const YCPMap &m) {
    ModuleEntry::EntryArg ret_map;
    YCPMapIterator it = m->begin ();

    for (; it != m->end(); ++it)
	if (it.key()->isString () && it.value ()->isString ())
	    ret_map[VAL2STR(it.key ())] = VAL2STR(it.value ());
	else {
	    y2error("Map element must be string!");
	    return ModuleEntry::EntryArg();
	}
    
    return ret_map;
}


**
 * Dir
 *
YCPValue ModulesAgent::Dir(const YCPPath& path) {
  YCPList list;
  string elem;
  
    if (modules_conf == NULL)
	Y2_RETURN_VOID("Can't execute Dir before being mounted.")

    switch (path->length ()) {
    case 0:
	return map2list (modules_conf->getDirectives ());
    case 1:
        if(PC(0) == MAGIC_DIRECTIVE)
	    Y2_RETURN_VOID("Dir() doesn't support the .%s directive", MAGIC_DIRECTIVE)
	return map2list (modules_conf->getModules(PC(0)));
    }
  
    Y2_RETURN_VOID("Wrong path '%s' in Dir().", path->toString().c_str())
}


**
 * Read
 *
YCPValue ModulesAgent::Read(const YCPPath &path, const YCPValue& arg) {
	    
    if (modules_conf == NULL)
	Y2_RETURN_VOID("Can't execute Read before being mounted.")
	    
    switch (path->length ()) {
    case 0:
	return YCPList (map2list (modules_conf->getDirectives()) );
    case 1:
	if (arg.isNull ())
	    return YCPList (map2list (modules_conf->getModules(PC(0))) );
	if (arg->isString ()) {
	    if (PC(0) == "options")
		return YCPMap (map2ycpmap (modules_conf->getOptions(VAL2STR(arg))));
	    else
		return YCPString (modules_conf->getArgument(PC(0), VAL2STR(arg)));
	}
	
    case 2:
	if (!arg.isNull () && arg->isString ()) {
	    if (PC(1) == "comment")
		return YCPString (modules_conf->getComment(PC(0), VAL2STR(arg)));
	    if (PC(0) == "options")
		return YCPString (modules_conf->getOption(VAL2STR(arg), PC(1)));
	}
    }
    
    Y2_RETURN_VOID("Wrong path '%s' in Read().", path->toString().c_str())
}

**
 * Write
 *
YCPValue ModulesAgent::Write(const YCPPath &path, const YCPValue& value, const YCPValue& arg) {

    if (modules_conf == NULL)
	Y2_RETURN_VOID("Can't execute Write before being mounted.")

    if (path->isRoot() && value->isVoid ())
	return YCPBoolean(modules_conf->writeFile());

    switch (path->length ()) {

    case 1:
	if (!arg.isNull () && arg->isString ()) {
	    if (value->isVoid ())
		return YCPBoolean (modules_conf->removeEntry (PC(0), VAL2STR(arg)));
	    if (PC(0) == "options") {
		if (value->isMap ())
		    return YCPBoolean (modules_conf->setOptions(VAL2STR(arg),
								ycpmap2map (value->asMap ()),
								ModuleEntry::SET));
		else 
		    Y2_RETURN_YCP_FALSE("Argument for Write () not map.")
	    }
	    return YCPBoolean (modules_conf->setArgument (PC(0), VAL2STR(arg),
							  VAL2STR(value),
							  ModuleEntry::SET));
	}
	else 
	    Y2_RETURN_YCP_FALSE("Argument (2nd) for Write() is not string.")
		
    case 2:
	if (value->isString () && !arg.isNull () && arg->isString ()) {
	    if (PC(0) == "options")
		return YCPBoolean (modules_conf->setOption (VAL2STR(arg), PC(1),
							    VAL2STR(value),
							    ModuleEntry::SET));
	    if (PC(1) == "comment")
		return YCPBoolean (modules_conf->setComment (PC(0), VAL2STR(arg),
							     VAL2STR(value),
							     ModuleEntry::SET));
	}
	else
	    Y2_RETURN_YCP_FALSE("One or both Arguments for Write() is not string.")
    }
	
    Y2_RETURN_VOID("Wrong path '%s' in Write().", path->toString().c_str())
}


**
 * otherCommand
 *
YCPValue ModulesAgent::otherCommand(const YCPTerm& term) {
    string sym = term->symbol()->symbol();

    if (sym == "ModulesConf" && term->size() == 1) {
	if (term->value(0)->isString()) {
	    YCPString s = term->value(0)->asString();
	    if (modules_conf != NULL)
		delete modules_conf;
	    modules_conf = new ModulesConf(s->value());
	    return YCPVoid();
	} else 
	    Y2_RETURN_VOID("Bad first arg of ModulesConf(): is not a string.")
    }

    return YCPNull();
}*/

