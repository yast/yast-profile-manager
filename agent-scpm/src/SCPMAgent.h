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
#include <scr/SCRInterpreter.h>

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
    /**
     * Agent private variables
     */
    SCPM *scpm;
    int options;
    ofstream output, hash;
    string changesfile, tmpfile;    
    static void *call_prepare( SCPMAgent *);
    static void *call_switch( SCPMAgent *);
    static void *call_add( SCPMAgent *);
    static void *call_copy( SCPMAgent *);
    static void *call_enable( SCPMAgent *);
    bool auto_switch;
    pthread_t pt;
    switch_info_t switch_info;
    string profile, dest_profile;

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
			  const YCPValue& arg = YCPNull());

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
