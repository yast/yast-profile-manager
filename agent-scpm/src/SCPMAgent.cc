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
    if (scpm) delete scpm;
}

/**
 * Dir
 */
YCPValue SCPMAgent::Dir(const YCPPath& path)
{
    y2error("Wrong path '%s' in Read().", path->toString().c_str());
    return YCPVoid();
}

/**
 * Read
 */
YCPValue SCPMAgent::Read(const YCPPath &path, const YCPValue& arg)
{
    y2error("Path in Read(): %s", path->toString().c_str());
    

    if (path->length() == 0) {
    	return YCPString("0");
    }
    
    if (path->length() == 1) {
        
   	if (PC(0) == "profiles") {

        if (arg.isNull ()) {
        
    	    vector<string> l;
	        if ( !scpm->List( l ) ) y2error ("error");
                 //die( scpm_error, ERR_SUBSYS );
            else
                return YCPList(tolist(l));
        }
       	if (arg->isString ()) {
        	return arg;
	    }

    }
    
   	if (PC(0) == "resources") {
     
        vector<string> predefined, individual;
        YCPList sets;
        
        if (!scpm->ListResourceSets(predefined, individual)) 
            y2error ("error");
        else {
            sets->add (tolist(predefined));
            sets->add (tolist(individual));
            return sets;
        }

    }
	
    if (PC(0) == "progress")
        return YCPString("progress");
    
	if (PC(0) == "hash")
        return YCPString("hash");

    else
        return YCPString("1");
    
    }
    
    if (path->length() == 2) {
        
   	if ((PC(0) == "status") && (PC(1) == "enabled")) {
        if (!scpm->Status(scpm_status)) 
            y2error ("error");
        else
            return YCPBoolean(scpm_status.enabled);
    }

	if (PC(0) == "profiles") {
      if (PC(1) == "current") {
        string profile;

        if (!scpm->Active(profile)) 
            y2error ("error");
        else
            return YCPString(profile);
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
            y2error ("error(g)");
        else
            return YCPString(result);
      }
          
    
    }
     
   	if ((PC(0) == "resources") && (PC(1) == "current")) {
        string set;
        if (!scpm->GetResourceSet(set)) 
            y2error ("error");
        else
            return YCPString(set);
    }
    
    else
        return YCPString("??");
    }
    

    return YCPVoid();
}

/**
 * Write
 */
YCPValue SCPMAgent::Write(const YCPPath &path, const YCPValue& value,
			     const YCPValue& arg)
{
    y2error("Wrong path '%s' in Write().", path->toString().c_str());
    
    if (path->length() == 2) {
        
   	if ((PC(0) == "status") && (PC(1) == "enabled")) {

        bool action;

        if (!arg.isNull ()) {
            y2error ("error");
            return YCPVoid();
        }
        
        if (!arg->isBoolean ()) {
            y2error ("error");
            return YCPVoid();
        }

        action = arg->asBoolean()->value();
        
        if (action) {
            if (!scpm->Enable()) 
                y2error ("error");
            else
                return YCPBoolean(1);
        }
        else {
            if (!scpm->Disable()) 
                y2error ("error");
            else
                return YCPBoolean(1);
        }

    }

    }

    return YCPVoid();
}

/**
 * 
 */
YCPValue SCPMAgent::Execute(const YCPPath &path, const YCPValue& value,
			     const YCPValue& arg)
{
    y2error("Path in Read(): %s", path->toString().c_str());
    

    if (path->length() == 0) {
    	return YCPString("0");
    }
    
    if (path->length() == 1) {
    
   	if (PC(0) == "switch") {
     
        vector<string> predefined, individual;
        YCPList sets;
        
        if (!scpm->ListResourceSets(predefined, individual)) 
            y2error ("error");
        else {
            sets->add (tolist(predefined));
            sets->add (tolist(individual));
            return sets;
        }

    }
    }
    
    if (path->length() == 2) {
    
   	if (PC(0) == "profiles") {
      if (PC(1) == "add") {
        string profile;
        
        if (!arg.isNull ()) {
            y2error ("error");
            return YCPVoid();
        }
        else
            profile = arg->asString()->value();
        
        if (!scpm->Add(profile)) // auto_switch!!!
            y2error ("error");
        else
            return YCPBoolean(1);
      }
      
      if (PC(1) == "delete") {
        string profile;
        
        if (!arg.isNull ()) {
            y2error ("error");
            return YCPVoid();
        }
        else
            profile = arg->asString()->value();
        
        if (!scpm->Delete(profile)) 
            y2error ("error");
        else
            return YCPBoolean(1);
      }

      if (PC(1) == "copy") {
        string profile, source_profile;
        
        if (!arg.isNull ()) {
            y2error ("error");
            return YCPVoid();
        }
        else if (!arg->isList ()) { // osetrit velikost!
            y2error ("error");
            return YCPVoid();
        }
        profile = arg->asList()->value(0)->asString()->value(); 
        source_profile = arg->asList()->value(1)->asString()->value(); 
        
        if (!scpm->Copy(profile, source_profile)) 
            y2error ("error");
        else
            return YCPBoolean(1);
      }

      if (PC(1) == "rename") {
        string profile, new_profile;
        
        if (!arg.isNull ()) {
            y2error ("error");
            return YCPVoid();
        }
        else if (!arg->isList ()) { // osetrit velikost!
            y2error ("error");
            return YCPVoid();
        }
        profile = arg->asList()->value(0)->asString()->value(); 
        new_profile = arg->asList()->value(1)->asString()->value(); 
        
        if (!scpm->Rename(profile, new_profile)) 
            y2error ("error");
        else
            return YCPBoolean(1);
      }
      
      
    }
    }
     
    return YCPVoid();
}

/**
 * otherCommand
 */
YCPValue SCPMAgent::otherCommand(const YCPTerm& term)
{
    string sym = term->symbol()->symbol();

    if (sym == "SCPMAgent") {
        
        /* Your initialization */
        
        scpm = new SCPM (options);
        
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

