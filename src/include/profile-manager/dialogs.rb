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

# File:	include/profile-manager/dialogs.ycp
# Package:	Configuration of profile-manager
# Summary:	Dialogs definitions
# Authors:	Jiri Suchomel <jsuchome@suse.cz>
#
# $Id$
module Yast
  module ProfileManagerDialogsInclude
    def initialize_profile_manager_dialogs(include_target)
      Yast.import "UI"

      textdomain "profile-manager"

      Yast.import "FileUtils"
      Yast.import "Label"
      Yast.import "Popup"
      Yast.import "ProfileManager"
      Yast.import "Report"
      Yast.import "Wizard"

      Yast.include include_target, "profile-manager/helps.rb"
    end

    # popup for entering new service for resource group
    # @return service name; empty string on cancel
    def GetService
      # read the list of services...
      services = Convert.to_map(SCR.Read(path(".init.scripts.comments")))
      service_items = []

      Builtins.foreach(
        Convert.convert(services, :from => "map", :to => "map <any, map>")
      ) do |k, v|
        service_items = Builtins.add(service_items, Item(Id(k), k, false))
      end

      service = ""
      UI.OpenDialog(
        Opt(:decorated),
        HBox(
          HSpacing(1),
          VBox(
            VSpacing(0.2),
            # combo box label
            ComboBox(Id(:services), _("Available &Services"), service_items),
            VSpacing(0.2),
            HBox(
              PushButton(Id(:ok), Opt(:default), Label.OKButton),
              PushButton(Id(:cancel), Label.CancelButton)
            ),
            VSpacing(0.2)
          ),
          HSpacing(1)
        )
      )

      UI.SetFocus(Id(:services))

      ret = nil
      begin
        ret = UI.UserInput
        service = Convert.to_string(UI.QueryWidget(Id(:services), :Value))

        if ret == :cancel
          service = ""
        elsif ret == :ok
          # check service existence
          # TODO it has to exist, if combo is not editable
          if service != "" &&
              !Convert.to_boolean(
                SCR.Read(path(".init.scripts.exists"), service)
              )
            # yes/no popup
            if !Popup.YesNo(
                Builtins.sformat(
                  _("Service %1 does not exist.\nReally add it?"),
                  service
                )
              )
              next
            end
          end
        end
      end while ret != :ok && ret != :cancel

      UI.CloseDialog
      service
    end

    # popup for entering new file for resource group
    # @return file name; empty string on cancel
    def GetFile
      filename = ""
      UI.OpenDialog(
        Opt(:decorated),
        HBox(
          HSpacing(1),
          VBox(
            VSpacing(0.2),
            HBox(
              TextEntry(Id(:file), Label.FileName, ""),
              HSpacing(1),
              VBox(
                Label(""),
                # button label
                PushButton(Id(:browse), Opt(:key_F6), _("&Browse..."))
              )
            ),
            VSpacing(0.2),
            HBox(
              PushButton(Id(:ok), Opt(:default), Label.OKButton),
              PushButton(Id(:cancel), Label.CancelButton)
            ),
            VSpacing(0.2)
          ),
          HSpacing(1)
        )
      )

      UI.SetFocus(Id(:file))

      ret = nil
      begin
        ret = UI.UserInput
        filename = Convert.to_string(UI.QueryWidget(Id(:file), :Value))

        if ret == :cancel
          filename = ""
        elsif ret == :browse
          f = UI.AskForExistingFile(filename != "" ? "/" : filename, "*", "")
          filename = f if f != nil
          UI.ChangeWidget(Id(:file), :Value, filename)
        elsif ret == :ok
          if filename != "" && !FileUtils.Exists(filename)
            # error popup
            Popup.Error(_("File does not exist."))
            next
          end
        end
      end while ret != :ok && ret != :cancel

      UI.CloseDialog
      filename
    end


    # Popup for editing or adding resource group settings
    # @param [Hash] group map with info of the group
    # @return modified group
    def ResourceGroupPopup(group, resource_groups)
      group = deep_copy(group)
      resource_groups = deep_copy(resource_groups)
      groupname = Ops.get_string(group, "name", "")
      descr = Ops.get_string(group, "description", "")
      what = group == {} || Ops.get_string(group, "what", "") == "added" ? "added" : "edited"
      items = []
      org_group = Builtins.eval(group)

      # map for saving user_defined flags of each resource
      user_defined = {}

      # resource type (menu entry)
      filestring = _("File")
      # resource type (menu entry)
      servicestring = _("Service")
      type2string = { "file" => filestring, "service" => servicestring }
      string2type = { filestring => "file", servicestring => "service" }

      # Scan table items for presence of given resource
      has_resource = lambda do |name, type|
        translated = Ops.get_string(type2string, type, type)
        Builtins.contains(items, Item(Id(name), "X", name, translated)) ||
          Builtins.contains(items, Item(Id(name), "", name, translated))
      end
      # helper function for building table items of one resource group
      resource_group_items = lambda do
        Builtins.maplist(Ops.get_list(group, "resources", [])) do |res|
          name = Ops.get_string(res, "name", "")
          if Ops.get_boolean(res, "active", false)
            next Item(
              Id(name),
              "X",
              name,
              Ops.get_string(type2string, Ops.get_string(res, "type", ""), "")
            )
          else
            next Item(
              Id(name),
              "",
              name,
              Ops.get_string(type2string, Ops.get_string(res, "type", ""), "")
            )
          end
        end
      end

      con = HBox(
        VSpacing(26),
        HSpacing(1.5),
        VBox(
          HSpacing(60),
          VSpacing(0.5),
          # textentry label
          TextEntry(Id(:name), _("Resource &Group"), groupname),
          VSpacing(0.5),
          # textentry label
          TextEntry(Id(:descr), _("D&escription"), descr),
          VSpacing(0.5),
          # frame label
          Frame(
            _("Resources"),
            HBox(
              HSpacing(1),
              VBox(
                VSpacing(0.2),
                Table(
                  Id(:resources),
                  Opt(:notify),
                  Header(
                    # Header of the table with resources of one group 1/3
                    _("Active"),
                    # Header of the table with resources of one group 2/3
                    _("Name"),
                    # Header of the table with resources of one group 3/3
                    _("Type")
                  ),
                  items
                ),
                VSpacing(0.2),
                HBox(
                  # Push button label
                  PushButton(Id(:adds_b), Opt(:key_F3), _("Add &Service")),
                  # Push button label
                  PushButton(Id(:addf_b), Opt(:key_F4), _("Add &File")),
                  Left(
                    # Push button label
                    PushButton(Id(:delete_b), Opt(:key_F5), _("&Delete"))
                  ),
                  Right(
                    # Push button label
                    PushButton(Id(:reset_b), Opt(:key_F6), _("&Reset Group"))
                  )
                ),
                VSpacing(0.2)
              ),
              HSpacing()
            )
          ),
          VSpacing(0.5),
          HBox(
            HSpacing(),
            Left(PushButton(Id(:ok), Opt(:default), Label.OKButton)),
            Right(PushButton(Id(:cancel), Label.CancelButton)),
            HSpacing()
          ),
          VSpacing(0.5)
        ),
        HSpacing(1.5)
      )

      UI.OpenDialog(Opt(:decorated), con)

      # create the table items
      items = resource_group_items.call
      UI.ChangeWidget(Id(:resources), :Items, items)
      UI.SetFocus(Id(:name))

      Builtins.y2debug("edited group: %1", group)
      if Ops.get_boolean(group, "user_defined", true) == false
        # default group cannot be renamed
        UI.ChangeWidget(Id(:name), :Enabled, false)
      else
        # user defined group cannot be reset
        UI.ChangeWidget(Id(:reset_b), :Enabled, false)
      end

      ret = nil
      begin
        ret = Convert.to_symbol(UI.UserInput)
        groupname = Convert.to_string(UI.QueryWidget(Id(:name), :Value))
        selected = Convert.to_string(
          UI.QueryWidget(Id(:resources), :CurrentItem)
        )

        if ret == :resources
          items = Builtins.maplist(items) do |i|
            next deep_copy(i) if Ops.get_string(i, 2, "") != selected
            if Ops.get_string(i, 1, "") == "X"
              next Item(Id(selected), "", selected, Ops.get_string(i, 3, ""))
            else
              next Item(Id(selected), "X", selected, Ops.get_string(i, 3, ""))
            end
          end
          UI.ChangeWidget(Id(:resources), :Items, items)
          UI.ChangeWidget(Id(:resources), :CurrentItem, selected)
        end
        if ret == :reset_b
          orig_group = ProfileManager.GetDefaultResourceGroup(groupname)
          if Ops.greater_than(Builtins.size(orig_group), 0)
            Ops.set(
              group,
              "resources",
              Builtins.eval(Ops.get_list(orig_group, "resources", []))
            )
            Ops.set(
              group,
              "description",
              Builtins.eval(Ops.get_string(orig_group, "description", ""))
            )
            items = resource_group_items.call
            UI.ChangeWidget(Id(:resources), :Items, items)
            UI.ChangeWidget(
              Id(:descr),
              :Value,
              Ops.get_string(group, "description", "")
            )
          end
        end
        if ret == :delete_b
          Builtins.y2debug("deleting %1 from group %2", selected, groupname)
          # delete entry from local items
          items = Builtins.filter(items) do |i|
            Ops.get_string(i, 2, "") != selected
          end
          UI.ChangeWidget(Id(:resources), :Items, items)
        end
        if ret == :adds_b
          service = GetService()
          if service != ""
            if has_resource.call(service, "service")
              # message popup
              Popup.Message(
                Builtins.sformat(
                  _(
                    "Service\n" +
                      "%1\n" +
                      "is already present in this resource group."
                  ),
                  service
                )
              )
            else
              Builtins.y2debug(
                "new service %1 for group %2",
                service,
                groupname
              )
              items = Builtins.add(
                items,
                Item(
                  Id(service),
                  "",
                  service,
                  Ops.get_string(type2string, "service", "service")
                )
              )
              Ops.set(user_defined, service, true)
              UI.ChangeWidget(Id(:resources), :Items, items)
            end
          end
        end
        if ret == :addf_b
          file = GetFile()
          if file != ""
            if has_resource.call(file, "file")
              # message popup
              Popup.Message(
                Builtins.sformat(
                  _(
                    "File\n" +
                      "%1\n" +
                      "is already present in this resource group."
                  ),
                  file
                )
              )
            else
              Builtins.y2debug("new file %1 for group %2", file, groupname)
              items = Builtins.add(
                items,
                Item(
                  Id(file),
                  "",
                  file,
                  Ops.get_string(type2string, "file", "file")
                )
              )
              Ops.set(user_defined, file, true)
              UI.ChangeWidget(Id(:resources), :Items, items)
            end
          end
        end
        if ret == :ok
          # check name contents
          if !ProfileManager.CheckNameValidity(groupname)
            # Popup text (wrong data):
            Popup.Message(
              _(
                "The name of resource group may contain only \n" +
                  "letters, digits, \"-\", \"_\", \".\", \"@\" and \"#\"\n" +
                  "and must begin with a letter or digit.\n" +
                  "Try again."
              )
            )
            next
          end
          # check possible name conflicts
          if groupname != Ops.get_string(group, "name", "")
            if Builtins.haskey(resource_groups, groupname)
              # error message, %1 is group name
              Popup.Error(
                Builtins.sformat(
                  _("Resource group %1 already exists.\nUse another name."),
                  groupname
                )
              )
              ret = :notok
              next
            end
            if Ops.get_string(group, "name", "") != ""
              Ops.set(group, "org_name", Ops.get_string(group, "name", ""))
            end
            what = "renamed" if what != "added"
            Ops.set(group, "name", groupname)
          end
          Ops.set(group, "description", UI.QueryWidget(Id(:descr), :Value))
          Ops.set(group, "resources", Builtins.maplist(items) do |i|
            {
              "active"       => Ops.get_string(i, 1, "") == "X",
              "name"         => Ops.get_string(i, 2, ""),
              "type"         => Ops.get_string(
                string2type,
                Ops.get_string(i, 3, ""),
                Ops.get_string(i, 3, "")
              ),
              "user_defined" => Ops.get_boolean(
                user_defined,
                Ops.get_string(i, 2, ""),
                false
              )
            }
          end)
          if what == "added"
            Ops.set(group, "user_defined", true)
            Ops.set(group, "active", true)
          end
          group_modified = false
          Builtins.foreach(
            Convert.convert(group, :from => "map", :to => "map <string, any>")
          ) { |k, v| group_modified = true if Ops.get(org_group, k) != v }
          if group_modified
            Ops.set(group, "user_modified", true)
            Ops.set(group, "what", what)
            Builtins.y2milestone("resource group %1 modified", groupname)
          else
            ret = :back
          end
        end
        group = {} if ret == :cancel || ret == :back
      end while !Builtins.contains([:ok, :next, :cancel, :back], ret)

      UI.CloseDialog

      deep_copy(group)
    end


    # Resource configuration dialog
    # @return [Object] Returned value from UserInput() call
    def ConfigureResourcesDialog
      resource_groups = ProfileManager.GetResourceGroups

      resource_groups_deleted = deep_copy(
        ProfileManager.resource_groups_deleted
      )

      # helper function for building table items with resource groups
      resource_groups_items = lambda do
        items2 = []
        Builtins.foreach(resource_groups) do |name, group|
          next if Ops.get_string(group, "what", "") == "deleted"
          desc = Ops.get_string(group, "description", "")
          if Ops.get_boolean(group, "active", false)
            items2 = Builtins.add(items2, Item(Id(name), "X", name, desc))
          else
            items2 = Builtins.add(items2, Item(Id(name), "", name, desc))
          end
        end
        deep_copy(items2)
      end

      # Marks selected resource group for deletion (will not be shown in list)
      # @param [String] groupname name of group
      mark_group_deleted = lambda do |groupname|
        resource_groups = Builtins.remove(resource_groups, groupname)
        if Ops.get_string(resource_groups, [groupname, "what"], "") != "added"
          resource_groups_deleted = Builtins.add(
            resource_groups_deleted,
            groupname
          )
        end
        Builtins.y2debug("resource group %1 marked for deletion", groupname)

        nil
      end

      # Changes "active" flag of selected resource group to opposite value
      # @param [String] groupname name of group
      # @return new value
      mark_group_active = lambda do |groupname|
        active = !Ops.get_boolean(resource_groups, [groupname, "active"], false)
        Ops.set(resource_groups, [groupname, "active"], active)
        Ops.set(resource_groups, [groupname, "user_modified"], true)
        active
      end
      # Updates resource_groups map with currently edited group
      change_group = lambda do |group|
        group = deep_copy(group)
        if Ops.get_string(group, "org_name", "") != ""
          resource_groups = Builtins.remove(
            resource_groups,
            Ops.get_string(group, "org_name", "")
          )
        end
        resource_groups = Builtins.add(
          resource_groups,
          Ops.get_string(group, "name", ""),
          group
        )

        nil
      end

      # Caption of the dialog:
      caption = _("Configuration of Resource Groups")

      contents = HBox(
        HSpacing(1.5),
        VBox(
          VSpacing(1),
          # frame label:
          Frame(
            _("Resource Group"),
            HBox(
              HSpacing(0.5),
              VBox(
                VSpacing(0.2),
                Table(
                  Id(:rgroups),
                  Opt(:notify),
                  Header(
                    # Header of the table with resource groups 1/3
                    _("Active"),
                    # Header of the table with resource groups 2/3
                    _("Name"),
                    # Header of the table with resource groups 3/3
                    _("Description")
                  ),
                  []
                ),
                VSpacing(0.2),
                HBox(
                  # Push button label
                  PushButton(Id(:add_button), Opt(:key_F3), _("&Add")),
                  # Push button label
                  PushButton(Id(:edit_button), Opt(:key_F4), _("&Edit")),
                  Left(
                    # Push button label
                    PushButton(Id(:delete_button), Opt(:key_F5), _("&Delete"))
                  ),
                  Right(
                    PushButton(
                      Id(:reset_button),
                      Opt(:key_F6),
                      # Push button label
                      _("&Reset All")
                    )
                  )
                ),
                VSpacing(0.2)
              ),
              HSpacing(0.5)
            )
          ),
          VSpacing(1)
        ),
        HSpacing(1.5)
      )

      Wizard.SetContentsButtons(
        caption,
        contents,
        Ops.get_string(@HELPS, "resources", ""),
        Label.CancelButton,
        Label.OKButton
      )

      items = resource_groups_items.call
      UI.ChangeWidget(Id(:rgroups), :Items, items)

      UI.SetFocus(Id(:rgroups)) if items != []

      ret = nil
      selected = ""
      modified = false
      while true
        ret = UI.UserInput
        selected = Convert.to_string(UI.QueryWidget(Id(:rgroups), :CurrentItem))
        if ret == :delete_button
          # %1 is resource group name
          if Popup.YesNo(Builtins.sformat(_("Delete group %1?"), selected))
            mark_group_deleted.call(selected)
            UI.ChangeWidget(Id(:rgroups), :Items, resource_groups_items.call)
            modified = true
          end
          next
        elsif ret == :add_button || ret == :edit_button
          group = {}
          if ret == :add_button
            group = ResourceGroupPopup({}, resource_groups)
          else
            group = ResourceGroupPopup(
              Ops.get(resource_groups, selected, {}),
              resource_groups
            )
          end
          if group != {}
            change_group.call(group)
            UI.ChangeWidget(Id(:rgroups), :Items, resource_groups_items.call)
            UI.ChangeWidget(
              Id(:rgroups),
              :CurrentItem,
              Ops.get_string(group, "name", "")
            )
            modified = true
          end
          next
        elsif ret == :reset_button
          rgs = ProfileManager.GetDefaultResourceGroups
          if Ops.greater_than(Builtins.size(rgs), 0)
            resource_groups = Builtins.eval(rgs)
            resource_groups_deleted = []
            UI.ChangeWidget(Id(:rgroups), :Items, resource_groups_items.call)
          end
        elsif ret == :rgroups
          group = Ops.get(resource_groups, selected, {})
          if !Ops.get_boolean(group, "active", false)
            # files from the resource group that are not available
            missing_files = []
            Builtins.foreach(Ops.get_list(group, "resources", [])) do |res|
              name = Ops.get_string(res, "name", "")
              if Ops.get_string(res, "type", "") == "file" &&
                  Ops.get_boolean(res, "active", false) &&
                  !FileUtils.Exists(name)
                missing_files = Builtins.add(
                  missing_files,
                  Ops.get_string(res, "name", "")
                )
              end
            end
            if missing_files != [] &&
                !Popup.AnyQuestionRichText(
                  Popup.NoHeadline,
                  # continue/cancel popup message, %1 is group name, %2 file(s)
                  Builtins.sformat(
                    _(
                      "The file <b>%2</b> from resource group <i>%1</i>\n" +
                        "is not available on the system.<br>\n" +
                        "The resource group is probably not installed.<br>\n" +
                        "Really select this resource group?"
                    ),
                    selected,
                    Builtins.mergestring(missing_files, "<br>")
                  ),
                  60,
                  9,
                  Label.YesButton,
                  Label.NoButton,
                  :focus_no
                )
              next
            end
          end
          active = mark_group_active.call(selected)
          UI.ChangeWidget(
            Id(:rgroups),
            term(:Item, selected, 0),
            active ? "X" : ""
          )
          modified = true
        elsif ret == :back ||
            ret == :cancel && Popup.ReallyAbort(ProfileManager.Modified)
          # throw away changes
          break
        elsif ret == :next
          if modified
            ProfileManager.resource_groups_modified = true

            # save the local maps...
            ProfileManager.resource_groups = Builtins.eval(resource_groups)
            ProfileManager.resource_groups_deleted = Builtins.eval(
              resource_groups_deleted
            )
          end
          break
        end
      end
      Convert.to_symbol(ret)
    end


    # Popup for editing user permission
    # @param user name (empty means we want to add new)
    # @param [String] permission ("switch" or "all")
    # @param map with other user names already used (to prevent conflicts)
    # @return
    def EditUserPopup(name, permission, conflicts)
      conflicts = deep_copy(conflicts)
      user = {}

      name_entry = Left(Label(Id(:name), name))
      if name == ""
        name_entry = ComboBox(
          Id(:name),
          Opt(:editable, :hstretch),
          # combo box label
          _("Username"),
          Builtins.filter(ProfileManager.GetUsernames) do |n|
            !Builtins.haskey(conflicts, n)
          end
        )
      end

      con = HBox(
        HSpacing(1.5),
        VBox(
          HSpacing(35),
          VSpacing(),
          name_entry,
          VSpacing(0.5),
          # frame label (for radiobutton group)
          Frame(
            _("Permission"),
            HBox(
              HSpacing(0.5),
              RadioButtonGroup(
                Id(:rb_perm),
                VBox(
                  VSpacing(0.5),
                  # radio button label
                  Left(
                    RadioButton(
                      Id("switch"),
                      _("&Switch Only"),
                      permission == "switch" || permission == ""
                    )
                  ),
                  # radio button label
                  Left(
                    RadioButton(
                      Id("all"),
                      _("&Everything"),
                      permission == "all"
                    )
                  ),
                  VSpacing(0.5)
                )
              )
            )
          ),
          VSpacing(0.5),
          HBox(
            HSpacing(1),
            Left(PushButton(Id(:ok), Opt(:default), Label.OKButton)),
            Right(PushButton(Id(:cancel), Label.CancelButton)),
            HSpacing(1)
          ),
          VSpacing(0.5)
        ),
        HSpacing(1.5)
      )

      UI.OpenDialog(Opt(:decorated), con)

      ret = nil
      begin
        ret = UI.UserInput

        user = {} if ret == :cancel
        if ret == :ok
          Ops.set(user, "name", UI.QueryWidget(Id(:name), :Value))
          if Ops.get_string(user, "name", "") == "" ||
              Builtins.issubstring(Ops.get_string(user, "name", ""), " ")
            # error popup (empty user name)
            Report.Error(_("Enter a valid username."))
            ret = :not_ok
            next
          end
          Ops.set(
            user,
            "permission",
            UI.QueryWidget(Id(:rb_perm), :CurrentButton)
          )
        end
      end while ret != :ok && ret != :cancel

      UI.CloseDialog

      deep_copy(user)
    end

    # Dialog for configuration of users able to run scpm
    def UsersDialog
      # Dialog caption label:
      caption = _("User Permissions Configuration")

      ret = nil
      modified = false

      users = ProfileManager.GetUsers

      trans = {
        # radio button label (user permission)
        "switch" => _("Switch Only"),
        # radio button label (user permission)
        "all"    => _("Everything")
      }
      names = {} #map with user names (to check conflicts)

      # helper for generating list of items
      users_items = lambda do
        names = {}
        Builtins.maplist(users) do |name, perm|
          Ops.set(names, name, 1)
          Item(Id(name), name, Ops.get_string(trans, perm, ""))
        end
      end

      contents = HBox(
        HSpacing(3),
        VBox(
          VSpacing(1),
          # frame label
          Frame(
            _("Users"),
            HBox(
              HSpacing(0.5),
              VBox(
                VSpacing(0.2),
                Table(
                  Id(:users),
                  Opt(:notify),
                  Header(
                    # Header of the table with users 1/2
                    _("Username"),
                    # Header of the table with users 2/2
                    _("Permissions")
                  ),
                  []
                ),
                VSpacing(0.2),
                HBox(
                  PushButton(Id(:add), Opt(:key_F3), Label.AddButton),
                  PushButton(Id(:edit), Opt(:key_F4), Label.EditButton),
                  Left(
                    # Push button label
                    PushButton(Id(:delete), Opt(:key_F5), _("&Delete"))
                  )
                ),
                VSpacing(0.2)
              ),
              HSpacing(0.5)
            )
          ),
          VSpacing()
        ),
        HSpacing(3)
      )

      Wizard.SetContentsButtons(
        caption,
        contents,
        Ops.get_string(@HELPS, "users", ""),
        Label.CancelButton,
        Label.OKButton
      )

      items = users_items.call
      UI.ChangeWidget(Id(:users), :Items, items)
      UI.SetFocus(Id(:users)) if items != []

      UI.ChangeWidget(Id(:edit), :Enabled, items != [])
      UI.ChangeWidget(Id(:delete), :Enabled, items != [])

      while true
        ret = UI.UserInput
        selected = Convert.to_string(UI.QueryWidget(Id(:users), :CurrentItem))

        if ret == :add || ret == :edit || ret == :users
          user = {}
          if ret == :add
            user = EditUserPopup("", "", names)
          else
            user = EditUserPopup(selected, Ops.get(users, selected, ""), {})
          end
          if user != {}
            Ops.set(
              users,
              Ops.get_string(user, "name", ""),
              Ops.get_string(user, "permission", "")
            )
            items = users_items.call
            UI.ChangeWidget(Id(:users), :Items, items)
            UI.ChangeWidget(
              Id(:users),
              :CurrentItem,
              Ops.get_string(user, "name", "")
            )
          end
        elsif ret == :delete
          users = Builtins.remove(users, selected)
          items = users_items.call
          UI.ChangeWidget(Id(:users), :Items, items)
        end
        if ret == :add || ret == :delete
          UI.ChangeWidget(Id(:edit), :Enabled, items != [])
          UI.ChangeWidget(Id(:delete), :Enabled, items != [])
        end
        if ret == :back ||
            ret == :cancel && Popup.ReallyAbort(ProfileManager.Modified)
          break
        end
        if ret == :next
          ProfileManager.SetUsers(users)
          break
        end
      end
      Convert.to_symbol(ret)
    end
  end
end
