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

# File:	include/profile-manager/wizards.ycp
# Package:	Configuration of profile-manager
# Summary:	Wizards definitions
# Authors:	Jiri Suchomel <jsuchome@suse.cz>
#
# $Id$
module Yast
  module ProfileManagerWizardsInclude
    def initialize_profile_manager_wizards(include_target)
      Yast.import "UI"

      textdomain "profile-manager"

      Yast.import "Sequencer"
      Yast.import "Wizard"

      Yast.include include_target, "profile-manager/complex.rb"
    end

    # Main sequence of profile-manager
    # @return sequence result
    def MainSequence
      aliases = { "main" => lambda { MainDialog() }, "resources" => lambda do
        ConfigureResourcesDialog()
      end, "users" => lambda(
      ) do
        UsersDialog()
      end }

      sequence = {
        "ws_start"  => "main",
        "main"      => {
          :abort     => :abort,
          :cancel    => :abort,
          :next      => :next,
          :resources => "resources",
          :users     => "users"
        },
        "resources" => { :abort => :abort, :cancel => :abort, :next => "main" },
        "users"     => { :abort => :abort, :cancel => :abort, :next => "main" }
      }

      ret = Sequencer.Run(aliases, sequence)
      deep_copy(ret)
    end

    # Whole configuration of profile-manager
    # @return sequence result
    def ProfileManagerSequence
      aliases = {
        "read"  => [lambda { ReadDialog() }, true],
        "main"  => lambda { MainSequence() },
        "write" => [lambda { WriteDialog() }, true]
      }

      sequence = {
        "ws_start" => "read",
        "read"     => { :abort => :abort, :next => "main" },
        "main"     => { :abort => :abort, :next => "write" },
        "write"    => { :abort => :abort, :next => :next }
      }

      Wizard.OpenNextBackDialog
      Wizard.HideAbortButton
      Wizard.SetDesktopTitleAndIcon("profile-manager")
      ret = Sequencer.Run(aliases, sequence)
      UI.CloseDialog

      Convert.to_symbol(ret)
    end
  end
end
