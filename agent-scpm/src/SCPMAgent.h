/* SCPMAgent.h
 *
 * SCPM agent implementation
 *
 * Authors: Jiri Suchomel <jsuchome@suse.cz>
 *
 * $Id$
 */

#ifndef _SCPMAgent_h
#define _SCPMAgent_h

#include <Y2.h>
#include <scr/SCRAgent.h>

using namespace std;

#include <iostream>
#include <scpm.h>

#include <string>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libintl.h>
#include <fstream>
#include <pthread.h>


/**
 * @short An interface class between YaST2 and SCPM Agent
 */
class SCPMAgent : public SCRAgent
{
private:
    SCPM *scpm;

    int options;
    bool auto_switch;
    bool use_rg;
    ofstream output, hash;
    string changesfile, tmpfile, hashfile;

    pthread_t pt;
    switch_info_t switch_info;
    string profile, dest_profile;

    // Forbid any scpm calls, when not initialized (Execute(.scpm)).
    bool initialized;

    /**
     * Functions to be called in new thread.
     */
    static void *call_prepare( SCPMAgent *);
    static void *call_switch( SCPMAgent *);
    static void *call_save( SCPMAgent *);
    static void *call_add( SCPMAgent *);
    static void *call_copy( SCPMAgent *);
    static void *call_enable( SCPMAgent *);
    // ----------------------
	
    /**
     * Converts c++ resource_info_t to YCP map
     */
    YCPMap tomap_ri (resource_info_t ri);

    /**
     * Converts YCP map to c++ resource_info_t
     */
    resource_info_t frommap_ri(YCPMap map);

    /**
     * Converts c++ vector of resource_info_t structures to YCP list of maps
     */
    YCPList tolist_modified_resources (vector<resource_info_t> l);

    /**
     * Converts YCP list of maps to c++ vector of resource_info_t structures
     */
    vector<resource_info_t> fromlist_modified_resources(YCPList list);
    
    /**
     * Converts switch_info_t structure to YCP map
     */
    YCPMap tomap_sw(switch_info_t sw);
    /**
     * Converts YCP map to switch_info_t structure
     */
    switch_info_t frommap_sw(YCPMap map);

    /**
     * Converts YCP list of maps to c++ vector of resource_group_t structures
     */
    vector<resource_group_t> fromlist_groups(YCPList list);

    /**
     * Converts c++ vector of resource_group_t structures to YCP list of maps
     */
    YCPList tolist_groups(vector<resource_group_t> l);

    /**
     * Converts YCP map to c++ structure of type resource_group_t 
     */
    resource_group_t frommap_rg(YCPMap map);

    /**
     * Converts c++ structure of type resource_group_t to YCP map
     */
    YCPMap tomap_rg(resource_group_t rgroup);

    /**
     * Converts YCP list of maps to c++ vector of resource_entry_t structures
     */
    vector<resource_entry_t> fromlist_re(YCPList list);
    
    /**
     * Converts YCP map to c++ structure of type resource_entry_t 
     */
    resource_entry_t frommap_re(YCPMap map);

    /**
     * Converts c++ structure of type resource_entry_t to YCP map
     */
    YCPMap tomap_re(resource_entry_t rentry);

public:
    /**
     * Default constructor.
     */
    SCPMAgent();

    /**
     * Destructor.
     */
    virtual ~SCPMAgent();

    /**
     * Provides SCR Read ().
     * @param path Path that should be read.
     * @param arg Additional parameter.
     */
    virtual YCPValue Read(const YCPPath &path,
			  const YCPValue& arg = YCPNull(),
			  const YCPValue& opt = YCPNull());

    /**
     * Provides SCR Write ().
     */
    virtual YCPValue Write(const YCPPath &path,
			   const YCPValue& value,
			   const YCPValue& arg = YCPNull());

    /**
     * Provides SCR Execute ().
     */
    virtual YCPValue Execute(const YCPPath &path,
			     const YCPValue& value = YCPNull(),
			     const YCPValue& arg = YCPNull());
                 

    /**
     * Provides SCR Dir ().
     */
    virtual YCPValue Dir(const YCPPath& path);

    /**
     * Used for mounting the agent.
     */
    virtual YCPValue otherCommand(const YCPTerm& term);
};

#endif /* _SCPMAgent_h */
