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

# File:	clients/profile-manager.ycp
# Package:	Configuration of profile-manager
# Summary:	Main file
# Authors:	Jiri Suchomel <jsuchome@suse.cz>
#
# $Id$
#
# Main file for profile-manager configuration. Uses all other files.
module Yast
  class ProfileManagerClient < Client
    def main
      Yast.import "UI"

      #**
      # <h3>Configuration of the profile-manager</h3>

      textdomain "profile-manager"

      Builtins.y2milestone("----------------------------------------")
      Builtins.y2milestone("Profile-manager module started")

      Yast.import "CommandLine"
      Yast.import "Confirm"
      Yast.import "PackageSystem"
      Yast.include self, "profile-manager/wizards.rb"


      # the command line description map
      @cmdline = {
        "id"         => "profile-manager",
        # translators: command line help text for Profile manage module
        "help"       => _(
          "Profile manager configuration module"
        ),
        "guihandler" => fun_ref(method(:Sequence), "any ()"),
        "initialize" => fun_ref(ProfileManager.method(:Read), "boolean ()"),
        "finish"     => fun_ref(ProfileManager.method(:Write), "boolean ()"),
        "actions"    => {
          "summary"   => {
            "handler" => fun_ref(
              method(:ProfileManagerSummaryHandler),
              "boolean (map)"
            ),
            # translators: command line help text for summary action
            "help"    => _(
              "Configuration summary of profile manager"
            )
          },
          "configure" => {
            "handler" => fun_ref(
              method(:ProfileManagerChangeConfiguration),
              "boolean (map)"
            ),
            # translators: command line help text for configure action
            "help"    => _(
              "Change the profile manager settings"
            )
          }
        },
        "options"    => {
          "users" => {
            # translators: command line help text for the kdc option
            "help"     => _(
              "Enable or disable profile switching for non-root users"
            ),
            "type"     => "enum",
            "typespec" => ["yes", "no"]
          }
        },
        "mappings"   => { "summary" => [], "configure" => ["users"] }
      }

      @ret = :auto

      if PackageSystem.CheckAndInstallPackagesInteractive(["scpm"])
        @ret = CommandLine.Run(@cmdline)
      end

      Builtins.y2debug("ret == %1", @ret)

      Builtins.y2milestone("Profile-manager module finished")
      Builtins.y2milestone("----------------------------------------")
      deep_copy(@ret)
    end

    # --------------------------------------------------------------------------
    # --------------------------------- cmd-line handlers

    # Change configuration of profile-manager
    # @param [Hash] options  a list of parameters passed as args
    # @return [Boolean] anything modified?
    def ProfileManagerChangeConfiguration(options)
      options = deep_copy(options)
      return false if !Builtins.haskey(options, "users")
      allowed = Ops.get_string(options, "users", "no") == "yes"
      if ProfileManager.users_allowed != allowed
        ProfileManager.users_allowed = allowed
        ProfileManager.users_modified = true
      end
      ProfileManager.users_modified
    end

    # Print summary of basic options
    # @return [Boolean] false
    def ProfileManagerSummaryHandler(options)
      options = deep_copy(options)
      # summary line
      CommandLine.Print(
        ProfileManager.enabled ?
          _("SCPM is enabled.") :
          # summary line
          _("SCPM is disabled.")
      )

      active_groups = []
      Builtins.foreach(ProfileManager.GetResourceGroups) do |name, group|
        if Ops.get_boolean(group, "active", false) &&
            Ops.get_string(group, "what", "") != "deleted"
          active_groups = Builtins.add(active_groups, name)
        end
      end
      if ProfileManager.enabled &&
          Ops.greater_than(Builtins.size(active_groups), 0)
        # summary item
        CommandLine.Print(
          Builtins.sformat(
            _("Active Resource Groups: %1"),
            Builtins.mergestring(active_groups, ",")
          )
        )
      end

      false
    end

    # run the wizard sequence
    def Sequence
      return :cancel if !Confirm.MustBeRoot
      ProfileManagerSequence()
    end
  end
end

Yast::ProfileManagerClient.new.main
