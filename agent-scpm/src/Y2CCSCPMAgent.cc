/* Y2CCSCPMAgent.cc
 *
 * SCPM agent implementation
 *
 * Authors: Jiri Suchomel <jsuchome@suse.cz>
 *
 * $Id$
 */

#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>
#include <scr/SCRInterpreter.h>

#include "SCPMAgent.h"

typedef Y2AgentComp <SCPMAgent> Y2SCPMAgentComp;

Y2CCAgentComp <Y2SCPMAgentComp> g_y2ccag_SCPM ("ag_scpm");
