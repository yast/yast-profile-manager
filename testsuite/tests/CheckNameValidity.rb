# encoding: utf-8

# Author:	Jiri Suchomel <jsuchome@suse.cz>
# $Id$
module Yast
  class CheckNameValidityClient < Client
    def main
      # testedfiles: ProfileManager.ycp
      Yast.import "Testsuite"
      Yast.import "ProfileManager"

      TEST_It_Now("new name")
      TEST_It_Now("new_name")
      TEST_It_Now("_new_name")

      nil
    end

    def TEST_It_Now(name)
      Testsuite.Dump("name to check:")
      Testsuite.Dump(name)
      Testsuite.Test(lambda { ProfileManager.CheckNameValidity(name) }, [
        {},
        {},
        {}
      ], nil)

      nil
    end
  end
end

Yast::CheckNameValidityClient.new.main
