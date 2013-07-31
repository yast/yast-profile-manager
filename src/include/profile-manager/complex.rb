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

# File:	include/profile-manager/complex.ycp
# Package:	Configuration of profile-manager
# Summary:	Dialogs definitions
# Authors:	Jiri Suchomel <jsuchome@suse.cz>
#
# $Id$
module Yast
  module ProfileManagerComplexInclude
    def initialize_profile_manager_complex(include_target)
      Yast.import "UI"

      textdomain "profile-manager"

      Yast.import "Label"
      Yast.import "Popup"
      Yast.import "ProfileManager"
      Yast.import "Wizard"

      Yast.include include_target, "profile-manager/helps.rb"
      Yast.include include_target, "profile-manager/dialogs.rb"
    end

    # Read settings dialog
    # @return `abort if aborted and `next otherwise
    def ReadDialog
      Wizard.RestoreHelp(Ops.get_string(@HELPS, "read", ""))
      ret = ProfileManager.Read
      ret ? :next : :abort
    end

    # Write settings dialog
    # @return `abort if aborted and `next otherwise
    def WriteDialog
      Wizard.RestoreHelp(Ops.get_string(@HELPS, "write", ""))
      ret = ProfileManager.Write
      ret ? :next : :abort
    end


    # Main dialog for scpm configuration
    def MainDialog
      # Dialog caption label:
      caption = _("SCPM Configuration")

      conf = deep_copy(ProfileManager.scpm_conf)

      resource_groups = ProfileManager.GetResourceGroups
      users_allowed = ProfileManager.users_allowed

      contents = HBox(
        HSpacing(3),
        VBox(
          VSpacing(1),
          # frame label:
          Frame(
            _("Status"),
            HBox(
              HSpacing(0.5),
              VBox(
                VSpacing(0.5),
                RadioButtonGroup(
                  Id(:rb_enabled),
                  VBox(
                    Left(
                      RadioButton(
                        Id(:ena),
                        Opt(:notify),
                        # Push button label (state of SCPM)
                        _("&Enabled")
                      )
                    ),
                    Left(
                      RadioButton(
                        Id(:dis),
                        Opt(:notify),
                        # Push button label (state of SCPM)
                        _("&Disabled")
                      )
                    )
                  )
                ),
                VSpacing(0.5)
              )
            )
          ),
          VSpacing(1),
          # frame label:
          Frame(
            _("Settings"),
            HBox(
              HSpacing(),
              VBox(
                VSpacing(0.5),
                HBox(
                  # combobox label
                  Left(
                    ComboBox(
                      Id("switch_mode"),
                      _("&Switch Mode"),
                      [
                        # switch mode
                        Item(Id("normal"), _("Normal")),
                        # switch mode
                        Item(Id("force"), _("Save Changes")),
                        # switch mode
                        Item(Id("skip"), _("Drop Changes"))
                      ]
                    )
                  ),
                  HSpacing(),
                  # combobox label
                  ComboBox(
                    Id("boot_mode"),
                    _("B&oot Mode"),
                    [
                      # switch mode (combo box item)
                      Item(Id("force"), _("Save Changes")),
                      # switch mode (combo box item)
                      Item(Id("skip"), _("Drop Changes"))
                    ]
                  )
                ),
                VSpacing(),
                Left(
                  CheckBox(
                    Id("verbose"),
                    # checkbox label
                    _("&Verbose Progress Messages"),
                    Ops.get_boolean(conf, "verbose", false)
                  )
                ),
                Left(
                  CheckBox(
                    Id("debug"),
                    # checkbox label
                    _("&Log Debug Messages"),
                    Ops.get_boolean(conf, "debug", false)
                  )
                ),
                HBox(
                  Left(
                    CheckBox(
                      Id(:users_allowed),
                      # checkbox label
                      _("Allow Profile Management for Non-root &Users"),
                      users_allowed
                    )
                  ),
                  # pushbutton label
                  PushButton(Id(:users), _("Co&nfigure..."))
                ),
                VSpacing(2)
              ),
              HSpacing()
            )
          ),
          VSpacing(),
          #frame label
          Frame(
            _("Resource Groups"),
            HBox(
              HSpacing(1),
              VBox(
                VSpacing(0.6),
                RichText(Id(:rt_res), ""),
                # button label
                Right(PushButton(Id(:resources), _("&Configure...")))
              ),
              HSpacing(1)
            )
          ),
          VSpacing(1)
        ),
        HSpacing(3)
      )

      Wizard.SetContentsButtons(
        caption,
        contents,
        Ops.get_string(@HELPS, "main", ""),
        Label.CancelButton,
        Label.OKButton
      )

      if ProfileManager.enabled
        UI.ChangeWidget(Id(:rb_enabled), :CurrentButton, :ena)
      else
        UI.ChangeWidget(Id(:rb_enabled), :CurrentButton, :dis)
        UI.ChangeWidget(Id(:resources), :Enabled, false)
      end

      UI.ChangeWidget(
        Id("switch_mode"),
        :Value,
        Ops.get_string(conf, "switch_mode", "normal")
      )
      UI.ChangeWidget(
        Id("boot_mode"),
        :Value,
        Ops.get_string(conf, "boot_mode", "force")
      )

      # rich text label (list of groups follows)
      active_rt = Builtins.sformat(_("<b>Active: </b>"))
      active_groups = []
      Builtins.foreach(resource_groups) do |name, group|
        if Ops.get_boolean(group, "active", false) &&
            Ops.get_string(group, "what", "") != "deleted"
          active_groups = Builtins.add(active_groups, name)
        end
      end
      if active_groups == []
        # rich text label (no group in the list)
        active_rt = Ops.add(active_rt, _("none"))
      else
        active_rt = Ops.add(
          active_rt,
          Builtins.mergestring(active_groups, ", ")
        )
      end
      UI.ChangeWidget(Id(:rt_res), :Value, active_rt)

      ret = nil
      while true
        ret = UI.UserInput
        if ret == :dis
          UI.ChangeWidget(Id(:resources), :Enabled, false)
          next
        elsif ret == :ena
          UI.ChangeWidget(Id(:resources), :Enabled, true)
          next
        elsif ret == :back || ret == :cancel || ret == :abort
          break if !ProfileManager.Modified
          break if Popup.ReallyAbort(true)
        elsif ret == :resources || ret == :next || ret == :users
          # give control to sequencer
          break
        end
      end

      if ret == :next || ret == :users || ret == :resources
        Builtins.foreach(
          Convert.convert(conf, :from => "map", :to => "map <string, any>")
        ) { |key, val| Ops.set(conf, key, UI.QueryWidget(Id(key), :Value)) }
        if conf != ProfileManager.scpm_conf
          ProfileManager.scpm_conf = deep_copy(conf)
          ProfileManager.conf_modified = true
        end
        users_allowed = Convert.to_boolean(
          UI.QueryWidget(Id(:users_allowed), :Value)
        )
        if users_allowed != ProfileManager.users_allowed
          ProfileManager.users_allowed = users_allowed
          ProfileManager.users_modified = true
        end

        if UI.QueryWidget(Id(:rb_enabled), :CurrentButton) == :ena
          if !ProfileManager.enabled
            ProfileManager.enabled = true
            ProfileManager.status_modified = !ProfileManager.status_modified
          end
        else
          if ProfileManager.enabled
            ProfileManager.enabled = false
            ProfileManager.status_modified = !ProfileManager.status_modified
          end
        end
        Builtins.y2milestone(
          "enabled: %1, status modified: %2, conf modified: %3",
          ProfileManager.enabled,
          ProfileManager.status_modified,
          ProfileManager.conf_modified
        )
      end
      deep_copy(ret)
    end
  end
end
