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

# File:	modules/ProfileManager.ycp
# Package:	Configuration of profile-manager
# Summary:	Data for configuration of profile-manager, input and output funcs.
# Authors:	Jiri Suchomel <jsuchome@suse.cz>
#
# $Id$
#
# Representation of the configuration of profile-manager.
# Input and output routines.
require "yast"

module Yast
  class ProfileManagerClass < Module
    def main
      Yast.import "UI"
      textdomain "profile-manager"

      Yast.import "Directory"
      Yast.import "FileUtils"
      Yast.import "Label"
      Yast.import "Mode"
      Yast.import "NetworkService"
      Yast.import "Package"
      Yast.import "Popup"
      Yast.import "Progress"
      Yast.import "Report"
      Yast.import "String"
      Yast.import "Users"
      Yast.import "UsersCache"


      # Is SCPM enabled?
      @enabled = false

      # Is SCPM re-initialization needed? (possibly after update)
      @needs_reinit = false

      # Is SCPM recovery? (after something failed)
      @needs_recover = false


      # Was enabled/disabled status changed?
      @status_modified = false

      # if resource groups were modified
      @resource_groups_modified = false

      # if SCPM DB needs to be saved (-> SCPM object desctructor)
      @save_db = false

      # if SCPM configuration (stored in scpm_conf map) was modified
      @conf_modified = false

      # if list of users was modified
      @users_modified = false

      # Is SCPM initialized?
      # This is set to true after first enabling.
      @initialized = false

      #  When scpm action fails, the error message is stored here by SetError()
      @scpm_error = ""

      # this map holds the contents of scpm.conf configuration file
      @scpm_conf = {}


      # all resource groups available; filled by ReadResourceGroups()
      @resource_groups = {}

      # list of resource groups marked for deletion
      @resource_groups_deleted = []

      # Map of users with permissions to run SCPM
      @users = {}

      # If users are available to switch profiles
      @users_allowed = false

      # List of user names, read from system via Users module
      @usernames = []

      # path to scpm qt front-end
      @sumfpath = "/usr/bin/sumf"

      # path to scpm bin
      @scpmpath = "/usr/sbin/scpm"

      # path to config file with users
      @userspath = "/etc/scpm.users"

      # The name of the file, where the progress state is beeing written.
      # (The same name must use the scpm agent!)
      @progressfile = "/scpm.progress"
      @hashfile = "/scpm.hash"
      @progresspath = ""
      @hashpath = ""
      @last_ret = ""

      # how many lines of progress has been pruducted
      @progress_size = 0

      # how many times was progress bar used
      @hash_size = 0
    end

    # Was anything modified?
    def Modified
      @conf_modified || @resource_groups_modified || @status_modified || @users_modified
    end

    # This function is called when some scpm command failed.
    # The error message is read and shown in Popup::Error.
    def ShowError
      err = Convert.to_string(SCR.Read(path(".scpm.error")))
      Popup.Error(err) if err != nil && err != ""

      nil
    end

    # This function is called when some scpm command failed.
    # The error message is read and stored in global variable scpm_error.
    def SetError
      err = Convert.to_string(SCR.Read(path(".scpm.error")))
      if err != nil && err != ""
        @scpm_error = err
      else
        @scpm_error = ""
      end

      nil
    end


    # Writes modified resource group.
    # @param [String] groupname resource group name
    # @param [Array] resources list of resources belonging to this group
    # @param [String] descr group description
    # @return success
    def SetResourceGroup(groupname, resources, descr)
      resources = deep_copy(resources)
      ret = SCR.Write(
        Builtins.add(path(".scpm.rg.group"), groupname),
        resources,
        descr
      )
      SetError() if !ret
      ret
    end

    # Gets resources of resource group.
    # @param [String] groupname resource group name
    # @return [Array] of resources
    def GetResourceGroupResources(groupname)
      resource_group = Convert.to_list(
        SCR.Read(path(".scpm.rg.group"), groupname)
      )
      if resource_group == nil
        SetError()
        return []
      end
      deep_copy(resource_group)
    end

    # Reads all available resource groups.
    # @return true on success
    def ReadResourceGroups
      @resource_groups = Convert.to_map(SCR.Read(path(".scpm.rg")))
      @resource_groups_deleted = []
      if @resource_groups == nil
        SetError()
        return false
      end
      Builtins.foreach(
        Convert.convert(
          @resource_groups,
          :from => "map",
          :to   => "map <string, map>"
        )
      ) do |name, group|
        Ops.set(
          @resource_groups,
          [name, "resources"],
          GetResourceGroupResources(name)
        )
      end

      true
    end

    def ReadResourceGroup(groupname)
      resource_group = Convert.to_map(
        SCR.Read(path(".scpm.rg.group_map"), groupname)
      )
      if resource_group == nil
        SetError()
        return {}
      end
      deep_copy(resource_group)
    end

    # get the default values for given resource group
    def GetDefaultResourceGroup(groupname)
      resource_group = Convert.to_map(
        SCR.Read(path(".scpm.rg.group_default"), groupname)
      )
      if resource_group == nil
        ShowError()
        return {}
      end
      deep_copy(resource_group)
    end

    # get the map of default resource groups
    def GetDefaultResourceGroups
      default_groups = Convert.convert(
        SCR.Read(path(".scpm.rg")),
        :from => "any",
        :to   => "map <string, map>"
      )
      if default_groups == nil
        ShowError()
        return {}
      end
      Builtins.foreach(default_groups) do |name, group|
        Ops.set(
          default_groups,
          name,
          Builtins.union(group, GetDefaultResourceGroup(name))
        )
      end

      deep_copy(default_groups)
    end

    # Resets RG's to default values
    # @return success
    def ResetResourceGroups
      ret = Convert.to_boolean(SCR.Execute(path(".scpm.rg.reset")))
      SetError() if !ret
      ret
    end

    # Resets one resource group to default values
    # @return success
    def ResetResourceGroup(groupname)
      ret = Convert.to_boolean(
        SCR.Execute(path(".scpm.rg.group.reset"), groupname)
      )
      SetError() if !ret
      ret
    end


    # Returns all available resource groups.
    # @return [Hash] of groups
    def GetResourceGroups
      Convert.convert(
        @resource_groups,
        :from => "map",
        :to   => "map <string, map>"
      )
    end

    # Read the set of users allowed to run SCPM
    def ReadUsers
      return false if !FileUtils.Exists(@userspath)

      @users = {}

      wholefile = Convert.to_string(
        SCR.Read(path(".target.string"), @userspath)
      )
      lines = Builtins.splitstring(wholefile, "\n")
      Builtins.foreach(lines) do |line|
        next if line == "" || Builtins.substring(line, 0, 1) == "#"
        ll = Builtins.splitstring(line, " \t")
        if Ops.greater_than(Builtins.size(ll), 1)
          Ops.set(@users, Ops.get_string(ll, 0, ""), Ops.get_string(ll, 1, ""))
        end
      end
      true
    end

    # Write new set of users allowed to run scpm
    def WriteUsers
      wholefile = Convert.to_string(
        SCR.Read(path(".target.string"), @userspath)
      )
      lines = Builtins.splitstring(wholefile, "\n")
      new_lines = Builtins.filter(lines) do |line|
        Builtins.substring(line, 0, 1) == "#"
      end
      Builtins.foreach(@users) do |name, perm|
        new_lines = Builtins.add(
          new_lines,
          Builtins.sformat("%1\t%2", name, perm)
        )
      end
      SCR.Write(
        path(".target.string"),
        @userspath,
        Builtins.mergestring(new_lines, "\n")
      )
    end

    # Return the map of users able to run scpm
    def GetUsers
      deep_copy(@users)
    end

    # Check if users are able to switch profiles
    # @return the result
    def GetUsersAllowed
      cmd = Ops.add("test -u ", @scpmpath)
      if FileUtils.Exists(@sumfpath)
        cmd = Ops.add(Ops.add(cmd, " -a -u "), @sumfpath)
      end
      SCR.Execute(path(".target.bash"), cmd) == 0
    end

    # Write the new status if the users are allowed to run SCPM or not
    # @return success of the action
    def WriteUsersAllowed
      return true if GetUsersAllowed() == @users_allowed # no change

      ret = true
      Builtins.foreach([@scpmpath, @sumfpath]) do |apppath|
        next if !ret || !FileUtils.Exists(apppath)
        cmd = Builtins.sformat(
          "/bin/chmod %1s %2",
          @users_allowed ? "+" : "-",
          apppath
        )
        out = Convert.to_map(SCR.Execute(path(".target.bash_output"), cmd))
        if Ops.get_string(out, "stderr", "") != ""
          Builtins.y2warning(
            "error calling %1: %2",
            cmd,
            Ops.get_string(out, "stderr", "")
          )
          ret = false
        end
      end
      ret
    end

    # Upodate the global map of users
    # @param map current map of users
    def SetUsers(current_users)
      current_users = deep_copy(current_users)
      @users_modified = true if !@users_modified && @users != current_users
      @users = Builtins.eval(current_users)
      @users_modified
    end

    # Return list of user names available in system
    # First time, read it using yast2-users, use cache for later calls.
    # @return [Array] of user names (for local users)
    def GetUsernames
      if @usernames == []
        # busy popup text
        Popup.ShowFeedback("", _("Reading list of users..."))
        gui = Users.GetGUI
        Users.SetGUI(false)
        Users.Read
        @usernames = UsersCache.GetUsernames("local")
        Users.SetGUI(gui)
        Popup.ClearFeedback
      end
      deep_copy(@usernames)
    end




    # Store error messages generated by multiple commands to one string
    def AllErrorMessages(error)
      if @scpm_error != ""
        error = Ops.add(error, "\n") if error != ""
        error = Ops.add(error, @scpm_error)
        @scpm_error = ""
      end
      error
    end

    # Save the resource groups edited in YaST UI
    # @return success
    def SaveResourceGroups
      @scpm_error = ""
      error = ""

      # deletion must be done in first cycle
      # (because some new group can have the name of deleted one)
      Builtins.foreach(
        Convert.convert(
          @resource_groups_deleted,
          :from => "list",
          :to   => "list <string>"
        )
      ) do |name|
        Builtins.y2milestone("deleting group %1", name)
        SetError() if !SCR.Write(path(".scpm.rg.group.delete"), name)
        # deactivate deleted group
        SCR.Write(path(".scpm.rg.group.activate"), name, false)
        error = AllErrorMessages(error)
      end
      @resource_groups_deleted = []
      # renaming must be done in second cycle
      # (because some new group can have the old name of renamed one)
      Builtins.foreach(
        Convert.convert(
          @resource_groups,
          :from => "map",
          :to   => "map <string, map <string, any>>"
        )
      ) do |name, group|
        if Ops.get_string(group, "what", "") == "renamed"
          Builtins.y2milestone(
            "renaming group %1 to %2",
            Ops.get_string(group, "org_name", ""),
            name
          )
          if !SCR.Write(
              Builtins.add(
                path(".scpm.rg.group.rename"),
                Ops.get_string(group, "org_name", name)
              ),
              name
            )
            SetError()
          end
        end
        error = AllErrorMessages(error)
      end
      # rest of modifications
      Builtins.foreach(
        Convert.convert(
          @resource_groups,
          :from => "map",
          :to   => "map <string, map <string, any>>"
        )
      ) do |name, group|
        what = Ops.get_string(group, "what", "")
        if what == "added" || what == "edited"
          Builtins.y2milestone("saving modified group %1", name)
          if !SCR.Write(
              Builtins.add(path(".scpm.rg.group.set"), name),
              Ops.get_list(group, "resources", []),
              Ops.get_string(group, "description", "")
            )
            SetError()
          end
        end
        # do Activation/Deactivation (it is not done in Set/Rename)
        if Ops.get_boolean(group, "user_modified", false)
          activate = Ops.get_boolean(group, "active", true)
          if !SCR.Write(path(".scpm.rg.group.activate"), name, activate)
            SetError()
          end
          Builtins.y2milestone(
            "changed active status of %1 to %2",
            name,
            activate
          )
        end
        error = AllErrorMessages(error)
      end
      @scpm_error = error if error != ""
      @scpm_error == ""
    end

    # reads a contents of scpm config file (/etc/scpm.conf)
    # @return false when file doesn't exist
    def ReadConfigFile
      return false if !FileUtils.Exists("/etc/sysconfig/scpm")

      @scpm_conf = {
        "debug"       => Convert.to_string(
          SCR.Read(path(".sysconfig.scpm.DEBUG"))
        ) == "yes",
        "verbose"     => Convert.to_string(
          SCR.Read(path(".sysconfig.scpm.VERBOSE"))
        ) == "yes",
        "switch_mode" => Convert.to_string(
          SCR.Read(path(".sysconfig.scpm.SWITCH_MODE"))
        ),
        "boot_mode"   => Convert.to_string(
          SCR.Read(path(".sysconfig.scpm.BOOT_MODE"))
        )
      }
      Builtins.y2debug("scpm configuration: %1", @scpm_conf)
      true
    end

    # writes configuration data to scpm config file (/etc/scpm.conf)
    # @return true
    def WriteConfigFile
      Builtins.foreach(
        Convert.convert(@scpm_conf, :from => "map", :to => "map <string, any>")
      ) do |key, value|
        agent = Builtins.add(path(".sysconfig.scpm"), Builtins.toupper(key))
        if Ops.is_boolean?(value)
          SCR.Write(agent, Convert.to_boolean(value) ? "yes" : "no")
        else
          SCR.Write(agent, Convert.to_string(value))
        end
      end
      true
    end


    # Initialize the SCPM agent.
    # @return true on success
    def Initialize
      # popup label, %1 is required application
      if !Package.InstallAllMsg(
          ["scpm"],
          _(
            "<p>To use the profile manager, the package <b>%1</b> is required.<br>\nInstall it now?</p>\n"
          )
        )
        return false
      end

      tmpdir = Convert.to_string(SCR.Read(path(".target.tmpdir")))
      @progresspath = Ops.add(tmpdir, @progressfile)
      @hashpath = Ops.add(tmpdir, @hashfile)
      ret = Convert.to_boolean(SCR.Execute(path(".scpm"), tmpdir))

      ret = false if ret == nil
      SetError() if ret == false

      ret
    end

    # Checks if SCPM is enabled/disabled/initialized.
    # @return true on success
    def ReadSCPMStatus
      # get the scpm_status_t struct as a map
      status = Convert.to_map(SCR.Read(path(".scpm.status")))
      if status == nil
        SetError()
        return false
      end

      @enabled = Ops.get_boolean(status, "enabled", false)
      @initialized = Ops.get_boolean(status, "initialized", false)
      @needs_reinit = @initialized &&
        Ops.get_boolean(status, "needs_reinit", false)
      @needs_recover = Ops.get_boolean(status, "needs_recover", false)
      true
    end

    # Calls SCPM::Recover (false) function, which replays journal after the crash
    # (started in separate agen'ts thread, return value must be checked with Wait)
    def Recover
      SCR.Execute(path(".scpm.recover"), {})

      nil
    end

    # Calls SCPM::Recover (true)
    def Rollback
      SCR.Execute(path(".scpm.rollback"), {})

      nil
    end



    # Ask user which action should be taken when scom recovery is necessary
    # @return [Symbol]: `cancel, `recover, `rollback
    def AskForRecoveryPopup
      ret = nil

      text =
        # popup text
        _(
          "SCPM is currently locked and needs recovery. This may be\n" +
            "the result of an aborted profile switch or similar. You\n" +
            "can have SCPM <b>Recover</b> the last command, which means your\n" +
            "database will be updated and outstanding actions will be\n" +
            "performed. You can also have SCPM <b>Rollback</b> the command, which means\n" +
            "the old system status will be restored if possible.\n"
        )

      UI.OpenDialog(
        Opt(:decorated),
        HBox(
          HSpacing(1.5),
          VSpacing(14),
          VBox(
            HSpacing(50),
            VSpacing(1),
            RichText(text),
            VSpacing(0.5),
            HBox(
              # Push button label
              PushButton(Id(:recover), Opt(:default), _("&Recover")),
              HSpacing(1),
              # Push button label
              PushButton(Id(:rollback), _("Ro&llback")),
              HSpacing(1),
              PushButton(Id(:cancel), Label.CancelButton)
            ),
            VSpacing(0.5)
          ),
          HSpacing(1.5)
        )
      )

      ret = Convert.to_symbol(UI.UserInput)
      UI.CloseDialog
      ret
    end

    # Checks the return value of agent's thread
    # @return [Boolean] true if thread finished succesfully
    def Wait
      ret = Convert.to_boolean(SCR.Read(path(".scpm.exit_status")))
      SetError() if !ret
      ret
    end

    # Gets the hash marks (to show in ProgressPopup) from the hashfile
    # @return number of characters in hashfile
    def GetHashMarks
      all = Convert.to_string(SCR.Read(path(".target.string"), @hashpath))
      return 0 if all == nil || all == ""
      Ops.subtract(Builtins.size(all), Ops.multiply(100, @hash_size))
    end

    # Reads the file with the progress informations.
    # @param [Boolean] everything if set to true, returns the entire rest of file
    # @return [String] the text to show in ProgressPopup (one line in normal case)
    def GetProgressText(everything)
      ret = ""
      all = Convert.to_string(SCR.Read(path(".target.string"), @progresspath))

      # check for non-existent file
      return ret if all == nil

      all_l = Builtins.splitstring(all, "\n")
      current = Ops.get_string(all_l, @progress_size, "")
      last = Ops.get_string(all_l, Ops.subtract(@progress_size, 1), "")

      if !everything # return only one line
        # last line has been updated:
        if @last_ret != "" && @last_ret != last
          ret = Builtins.substring(
            last,
            Builtins.size(@last_ret),
            Ops.subtract(Builtins.size(last), Builtins.size(@last_ret))
          )
          @last_ret = Ops.get_string(all_l, Ops.subtract(@progress_size, 1), "")
        # continue with the new line
        elsif Ops.less_than(@progress_size, Builtins.size(all_l)) &&
            current != "" &&
            current != "\n"
          ret = current
          if @last_ret != ""
            @last_ret = ret
            ret = Ops.add("\n", ret)
          else
            @last_ret = ret
          end
          @progress_size = Ops.add(@progress_size, 1)
        end # return everything to the end of file
      else
        if @last_ret != "" && @last_ret != last
          ret = Builtins.substring(
            last,
            Builtins.size(@last_ret),
            Ops.subtract(Builtins.size(last), Builtins.size(@last_ret))
          )
        end
        while Ops.less_than(@progress_size, Builtins.size(all_l)) &&
            current != "" &&
            current != "\n"
          ret = Ops.add("\n", current)
          @progress_size = Ops.add(@progress_size, 1)
          current = Ops.get_string(all_l, @progress_size, "")
        end
      end
      ret
    end

    # Popup for showing progress informations
    # @param [String] position of this progress:
    #  (if "first", popup is not closed, for "last" popup is not created)
    # @param [Fixnum] start start of progress (mainly 0)
    # @param [Fixnum] end end of progress (mainly 100)
    # @return [Boolean] true
    def ProgressPopup(position, start, _end)
      if position != "last"
        UI.OpenDialog(
          Opt(:decorated),
          HBox(
            HSpacing(1.5),
            VBox(
              HSpacing(60),
              VSpacing(1),
              # LogView label:
              LogView(Id(:progress), _("Progress Information"), 10, 0),
              VSpacing(1),
              ProgressBar(Id(:pb), "", 100, 0),
              VSpacing(1),
              HBox(
                Right(
                  PushButton(
                    Id(:close),
                    Opt(:default, :key_F10),
                    Label.CloseButton
                  )
                ),
                HSpacing(1)
              ),
              VSpacing(1)
            ),
            HSpacing(1.5)
          )
        )

        UI.ChangeWidget(Id(:close), :Enabled, false)
      end

      progresstext = ""
      hashmarks = start
      @last_ret = ""

      UI.BusyCursor
      begin
        Builtins.sleep(5)
        hashmarks = GetHashMarks()
        UI.ChangeWidget(Id(:pb), :Value, hashmarks)

        progresstext = GetProgressText(false)
        if progresstext != ""
          recoded = Convert.to_string(
            UI.Recode(WFM.GetEnvironmentEncoding, "UTF-8", progresstext)
          )
          UI.ChangeWidget(Id(:progress), :LastLine, recoded)
        end
      end while Ops.less_than(hashmarks, _end)

      # next hundred of hash marks
      if Ops.greater_or_equal(hashmarks, 100)
        @hash_size = Ops.add(@hash_size, 1)
      end

      # read to the end of file
      progresstext = GetProgressText(true)
      if progresstext != ""
        recoded = Convert.to_string(
          UI.Recode(WFM.GetEnvironmentEncoding, "UTF-8", progresstext)
        )
        UI.ChangeWidget(Id(:progress), :LastLine, recoded)
      end
      if position == "first" # hack: missing newline
        UI.ChangeWidget(Id(:progress), :LastLine, "\n")
      end

      UI.NormalCursor
      if position != "first"
        UI.ChangeWidget(Id(:close), :Enabled, true)
        UI.UserInput
        UI.CloseDialog
      end
      true
    end


    # Read all profile-manager settings
    # @return true on success
    def Read
      # profile-manager Read dialog caption:
      caption = _("Initializing Profile Manager Configuration")

      steps = 5

      Progress.New(
        caption,
        " ",
        steps,
        [
          # progress stage
          _("Initialize SCPM"),
          # progress stage
          _("Read the SCPM state"),
          # progress stage
          _("Read resource groups"),
          # progress stage
          _("Read SCPM settings")
        ],
        [
          # progress step
          _("Initializing SCPM..."),
          # progress step
          _("Reading the SCPM state..."),
          # progress step
          _("Reading resource groups..."),
          # progress step
          _("Reading SCPM settings..."),
          # progress step
          _("Finished")
        ],
        ""
      )

      return false if !NetworkService.ConfirmNetworkManager

      # initialize
      Progress.NextStage
      if !Initialize()
        # Error message: %1 is special error message
        Report.Error(
          Builtins.sformat(_("Cannot initialize SCPM:\n%1"), @scpm_error)
        )
        return false
      end

      # read status
      Progress.NextStage
      if !ReadSCPMStatus()
        # Error message: %1 is special error message
        Report.Error(
          Builtins.sformat(_("Cannot read SCPM status:\n%1"), @scpm_error)
        )


        return false
      end
      if @needs_reinit
        if Mode.commandline
          # error popup
          Report.Error(_("Reinitialization is needed."))
          return false
        end
        # yes/no popup
        if Popup.YesNo(
            _(
              "SCPM is currently locked because your system installation has changed.  \n" +
                "This is usually caused by a system update. Therefore, the data saved in\n" +
                "your profiles may be inconsistent, making it necessary to reinitialize SCPM. \n" +
                "The saved profiles will be lost and need to be set up again from scratch.  \n" +
                "Perform the reinitialization now?\n"
            )
          )
          # force enabling
          SCR.Write(path(".scpm.status.enabled.force"), true)
        end
      end

      if @needs_recover
        if Mode.commandline
          # error popup
          Report.Error(_("Recovery or rollback is needed."))
          return false
        end
        recover_ret = AskForRecoveryPopup()
        if recover_ret == :recover
          Recover()
        elsif recover_ret == :rollback
          Rollback()
        else
          return false
        end
        ProgressPopup("", 0, 100)
        if !Wait()
          ShowError()
          return false
        end
      end

      Progress.NextStage
      if !ReadResourceGroups()
        # error message (%1 is additional error message)
        Report.Error(
          Builtins.sformat(_("Cannot read resource groups:\n%1"), @scpm_error)
        )
      end

      # read user settings
      Progress.NextStage

      ReadConfigFile()

      @users_allowed = GetUsersAllowed()

      ReadUsers()

      Progress.NextStage
      # progress stage
      Progress.Title(_("Finished"))

      true
    end

    # Enables or disables SCPM
    # (depends on value of enabled variable).
    # @return true on success
    def WriteSCPMStatus
      ret = SCR.Write(path(".scpm.status.enabled"), @enabled)
      SetError() if !ret
      ret
    end

    # Enables SCPM for the first time
    def WriteStatusFirst
      SCR.Execute(path(".scpm.enable.first"))

      nil
    end

    # general function for writing status (enable/disable)
    def WriteStatus
      ret = true
      if @status_modified
        if !@initialized
          @initialized = true
          WriteStatusFirst()
          ProgressPopup("", 0, 100)
          if !Wait()
            @enabled = false
            ret = false
          end
        else
          ret = WriteSCPMStatus()
          @enabled = !@enabled if !ret
        end
      end
      if !ret
        # Error message. %1 is error report
        Report.Error(
          Builtins.sformat(
            _("Writing SCPM status was not successful:\n%1"),
            @scpm_error
          )
        )
      else
        @status_modified = false
        @save_db = true
      end
      ret
    end

    # Rebuilds SCPM database after changing resources.
    # @return true on success
    def RebuildDB
      return true if !@initialized # no need for rebuild

      ret = Convert.to_boolean(SCR.Execute(path(".scpm.resources.rebuild")))
      ShowError() if !ret
      ret
    end

    # function for writing modifications in resource groups
    def WriteResourceGroups
      ret = true
      if @resource_groups_modified
        ret = SaveResourceGroups()
        if ret
          RebuildDB()
          @resource_groups_modified = false
          @save_db = true
        else
          # Error popup text, %1 is additional error text
          Report.Error(
            Builtins.sformat(
              _("Saving resource groups failed:\n%1"),
              @scpm_error
            )
          )
        end
      end
      ret
    end

    # Write all profile-manager settings
    # @return true on success
    def Write
      # profile-manager Write dialog caption:
      caption = _("Writing Profile Manager Configuration")

      stagelist = [
        # progress stage:
        _("Write the SCPM database"),
        # progress stage:
        _("Write SCPM status"),
        # progress stage:
        _("Write resource groups"),
        # progress stage:
        _("Write SCPM settings")
      ]

      steplist = [
        # progress step:
        _("Writing the SCPM database..."),
        # progress step:
        _("Writing SCPM status..."),
        # progress step:
        _("Writing resource groups..."),
        # progress step:
        _("Writing SCPM settings..."),
        # progress step:
        _("Finished")
      ]

      Builtins.sleep(100)

      steps = Builtins.size(steplist)

      Progress.New(caption, " ", steps, stagelist, steplist, "")

      Progress.NextStage
      WriteStatus()

      Progress.NextStage
      WriteResourceGroups()

      Progress.NextStage
      if @save_db
        if !SCR.Write(path(".scpm"), nil)
          SetError()
          # Error message, %1 is extra error text
          Report.Error(
            Builtins.sformat(_("Cannot write SCPM database:\n%1"), @scpm_error)
          )
        else
          @save_db = false
        end
      end

      Progress.NextStage
      if @conf_modified && !WriteConfigFile()
        # Error message:
        Report.Error(_("Cannot write SCPM settings."))
      end

      if @users_modified
        WriteUsers()
        WriteUsersAllowed()
      end

      Progress.NextStage

      # progress stage
      Progress.Title(_("Finished"))

      true
    end

    # Checks if the name of the new profile consists of
    # valid characters: [a-zA-Z0-9_-.]
    # @param [String] name Name of the new profile.
    # @return true if valid
    def CheckNameValidity(name)
      return false if Ops.less_than(Builtins.size(name), 1)
      additional = "-_.@#" # additional characters allowed
      if name != Builtins.filterchars(name, Ops.add(String.CAlnum, additional))
        Builtins.y2milestone("invalid characters in profile name")
        return false
      end
      if Builtins.issubstring(additional, Builtins.substring(name, 0, 1))
        Builtins.y2milestone("profile name must start with letter or digit")
        return false
      end
      true
    end

    publish :variable => :enabled, :type => "boolean"
    publish :variable => :needs_reinit, :type => "boolean"
    publish :variable => :needs_recover, :type => "boolean"
    publish :variable => :status_modified, :type => "boolean"
    publish :variable => :resource_groups_modified, :type => "boolean"
    publish :variable => :save_db, :type => "boolean", :private => true
    publish :variable => :conf_modified, :type => "boolean"
    publish :variable => :users_modified, :type => "boolean"
    publish :variable => :initialized, :type => "boolean"
    publish :variable => :scpm_error, :type => "string"
    publish :variable => :scpm_conf, :type => "map"
    publish :variable => :resource_groups, :type => "map"
    publish :variable => :resource_groups_deleted, :type => "list"
    publish :variable => :users, :type => "map <string, string>", :private => true
    publish :variable => :users_allowed, :type => "boolean"
    publish :variable => :usernames, :type => "list <string>", :private => true
    publish :variable => :sumfpath, :type => "string", :private => true
    publish :variable => :scpmpath, :type => "string", :private => true
    publish :variable => :userspath, :type => "string", :private => true
    publish :variable => :progressfile, :type => "string", :private => true
    publish :variable => :hashfile, :type => "string", :private => true
    publish :variable => :progresspath, :type => "string", :private => true
    publish :variable => :hashpath, :type => "string", :private => true
    publish :variable => :last_ret, :type => "string", :private => true
    publish :variable => :progress_size, :type => "integer", :private => true
    publish :variable => :hash_size, :type => "integer"
    publish :function => :Modified, :type => "boolean ()"
    publish :function => :ShowError, :type => "void ()", :private => true
    publish :function => :SetError, :type => "void ()", :private => true
    publish :function => :SetResourceGroup, :type => "boolean (string, list, string)", :private => true
    publish :function => :GetResourceGroupResources, :type => "list (string)"
    publish :function => :ReadResourceGroups, :type => "boolean ()"
    publish :function => :ReadResourceGroup, :type => "map (string)"
    publish :function => :GetDefaultResourceGroup, :type => "map (string)"
    publish :function => :GetDefaultResourceGroups, :type => "map <string, map> ()"
    publish :function => :ResetResourceGroups, :type => "boolean ()"
    publish :function => :ResetResourceGroup, :type => "boolean (string)"
    publish :function => :GetResourceGroups, :type => "map <string, map> ()"
    publish :function => :ReadUsers, :type => "boolean ()"
    publish :function => :WriteUsers, :type => "boolean ()"
    publish :function => :GetUsers, :type => "map <string, string> ()"
    publish :function => :GetUsersAllowed, :type => "boolean ()"
    publish :function => :WriteUsersAllowed, :type => "boolean ()"
    publish :function => :SetUsers, :type => "boolean (map <string, string>)"
    publish :function => :GetUsernames, :type => "list <string> ()"
    publish :function => :AllErrorMessages, :type => "string (string)", :private => true
    publish :function => :SaveResourceGroups, :type => "boolean ()"
    publish :function => :ReadConfigFile, :type => "boolean ()"
    publish :function => :WriteConfigFile, :type => "boolean ()"
    publish :function => :Initialize, :type => "boolean ()", :private => true
    publish :function => :ReadSCPMStatus, :type => "boolean ()", :private => true
    publish :function => :Recover, :type => "void ()"
    publish :function => :Rollback, :type => "void ()"
    publish :function => :AskForRecoveryPopup, :type => "symbol ()", :private => true
    publish :function => :Wait, :type => "boolean ()"
    publish :function => :GetHashMarks, :type => "integer ()"
    publish :function => :GetProgressText, :type => "string (boolean)"
    publish :function => :ProgressPopup, :type => "boolean (string, integer, integer)"
    publish :function => :Read, :type => "boolean ()"
    publish :function => :WriteSCPMStatus, :type => "boolean ()"
    publish :function => :WriteStatusFirst, :type => "void ()"
    publish :function => :WriteStatus, :type => "boolean ()"
    publish :function => :RebuildDB, :type => "boolean ()"
    publish :function => :WriteResourceGroups, :type => "boolean ()"
    publish :function => :Write, :type => "boolean ()"
    publish :function => :CheckNameValidity, :type => "boolean (string)"
  end

  ProfileManager = ProfileManagerClass.new
  ProfileManager.main
end
