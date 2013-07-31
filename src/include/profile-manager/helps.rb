# encoding: utf-8

# ------------------------------------------------------------------------------
# Copyright (c) 2006-2012 Novell, Inc. All Rights Reserved.
#
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of version 2 of the GNU General Public License as published by the
# Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, contact Novell, Inc.
#
# To contact Novell about this file by physical or electronic mail, you may find
# current contact information at www.novell.com.
# ------------------------------------------------------------------------------

# File:	include/profile-manager/helps.ycp
# Package:	Configuration of profile-manager
# Summary:	Help texts of all the dialogs
# Authors:	Jiri Suchomel <jsuchome@suse.cz>
#
# $Id$
module Yast
  module ProfileManagerHelpsInclude
    def initialize_profile_manager_helps(include_target)
      textdomain "profile-manager"

      # All helps are here
      @HELPS = {
        # Read dialog help 1/1
        "read"      => _(
          "<p><b><big>Initializing Profile Manager Configuration</big></b><br>\nPlease wait...<br></p>\n"
        ),
        # Main dialog help 1/9
        "main"      => _(
          "<p><big><b>SCPM Configuration</b></big></p>\n<p>SCPM (System Configuration Profile Management) enables your system to save different configuration profiles and switch between them.</p>"
        ) +
          # main dialog help 2/9
          _(
            "<p>Activate and configure SCPM here.  Configure and switch SCPM profiles with the SUMF graphical interface or the <tt>scpm</tt> command.</p>"
          ) +
          # main dialog help - subsection heading
          _("<p><b>Status</b></p>") +
          # main dialog help 3/9
          _(
            "First, set the SCPM status to <b>Enabled</b>. If disabled later, none of the configuration data in the profiles will be lost. It just continues in the current configuration of your system and you cannot switch to any other profile until you enable it again.</p>"
          ) +
          # main dialog help - subsection heading
          _("<p><b>Settings</b></p>") +
          # main dialog help 4/9 (SCPM and SUMF are names of applications)
          _(
            "<p>The settings apply to the command line interface as well as to the graphical SCPM front-end named SUMF.</p>"
          ) +
          # main dialog help 5/9
          _(
            "<p><b>Switch Mode</b> defines the behavior of SCPM when switching\n" +
              "profiles. If set to <b>Normal</b> or <b>Save Changes</b>,\n" +
              "modified resources are preset for saving before the switch.\n" +
              "Setting it to <b>Drop Changes</b> results in presets for ignoring. The effect in the command line SCPM tool is more limited than in SUMF, so check the info page if you intend to use the SCPM tool.</p>\n"
          ) +
          # main dialog help 6/9
          _(
            "<p><b>Boot Mode</b> defines the behavior of SCPM at system boot time.\n" +
              "<b>Save Changes</b> applies all changes to the previous profile.\n" +
              "<b>Drop Changes</b> discards them.</p>\n"
          ) +
          # main dialog help 7/9
          _(
            "<p><b>Verbose Messages</b> affects the detail level of progress messages\nin the progress pop-up. If <b>Log Debug Messages</b> is set, SCPM writes debug messages to its log file (/var/log/scpm by default).</p>"
          ) +
          # main dialog help 8/9
          _(
            "<p>SCPM needs root privileges for operation. Check <b>Allow Profile Management for Non-root Users</b> and use <b>Configure</b> to set up users to authorize to use SCPM.</p>"
          ) +
          # main dialog help - subsection heading
          _("<p><b>Resource Groups</b></p>") +
          # main dialog help 9/9, i marks special term, use something appropriate for your language
          _(
            "<p>A configuration profile covers only the files and services to change\n" +
              "when switching to another profile. In SCPM terminology, these files and\n" +
              "services are called resources. Those resources are grouped into logical units,\n" +
              "called <i>resource groups</i>. SCPM comes with a predefined set of\n" +
              "groups handled by default. This is sufficient for most systems. \n" +
              "For a more detailed resource setup, click <b>Configure Resources</b>.</p>\n"
          ),
        # Configure Resources Dialog help 1/3
        "resources" => _(
          "<p><big><b>Configuring Resources</b></big></p>\n" +
            "<p>This list contains all installed resource groups.\n" +
            "A resource group usually represents a system service with all needed\n" +
            "configuration files. Select which services should be handled by the \n" +
            "profile management. Activate or deactivate the groups by\n" +
            "double-clicking them.</p>\n"
        ) +
          # Configure Resources Dialog help 2/3
          _(
            "<p>To achive an even more customized setup, create your own\n" +
              "resource groups or modify existing ones. With <b>Add</b> or <b>Edit</b>,\n" +
              "enter the resource group editor. It allows activation and\n" +
              "deactivation of resources and gives the possibility to create additional\n" +
              "resources and add them to your groups.</p>\n"
          ) +
          # Configure Resources Dialog help 3/3
          _(
            "<p>Return all resource groups to their initial states by pressing\n<b>Reset All</b>.</p>"
          ),
        # Users dialog help 1/3 (headline)
        "users"     => _(
          "<p><b><big>Users</big></b></p>"
        ) +
          # Users dialog help 2/3
          # "Switch Only" and "Everything" are values also used in dialogs.ycp (
          # see "radio button label (user permission)" comments
          _(
            "<p>To allow users of your system to perform profile\n" +
              "management operations, add them here. There are two levels of permission:\n" +
              "<b>Switch Only</b> means the specified user may switch profiles, but nothing else. <b>Everything</b> means the user may perform any operation,\n" +
              "including adding and removing profiles.</p>"
          ) +
          # users dialog help 3/3
          _(
            "<p>Use <b>Add</b>, <b>Edit</b>, and <b>Delete</b> to modify the list of users with special permissions.</p>"
          ),
        # Write dialog help 1/1
        "write"     => _(
          "<p><b><big>Writing Profile Manager Configuration</big></b><br>\nPlease wait...<br></p>\n"
        )
      } 

      # EOF
    end
  end
end
