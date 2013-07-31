# encoding: utf-8

# Author:	Jiri Suchomel <jsuchome@suse.cz>
# $Id$
module Yast
  class GetProgressTextClient < Client
    def main
      # testedfiles: ProfileManager.ycp
      Yast.import "Testsuite"
      Yast.import "ProfileManager"


      @READ = { "target" => { "string" => "Starting...\nWorking..." } }

      @READ_continue = {
        "target" => { "string" => "Starting...\nWorking... [done]\nFinished" }
      }

      @READ_nil = { "target" => { "string" => nil } }

      ProfileManager.progresspath = "/tmp/changes.out"
      ProfileManager.progress_size = 0
      ProfileManager.last_ret = ""

      Testsuite.Test(lambda { ProfileManager.GetProgressText(false) }, [
        @READ_nil,
        {},
        {}
      ], nil)

      Testsuite.Test(lambda { ProfileManager.GetProgressText(false) }, [
        @READ,
        {},
        {}
      ], nil)

      # in fact, the function has another settings than following one, but
      # it returns string with newlines - and this I cannot print from test (?)
      ProfileManager.last_ret = ""

      Testsuite.Test(lambda { ProfileManager.GetProgressText(false) }, [
        @READ,
        {},
        {}
      ], nil)

      Testsuite.Test(lambda { ProfileManager.GetProgressText(false) }, [
        @READ_continue,
        {},
        {}
      ], nil)
      ProfileManager.last_ret = ""
      Testsuite.Test(lambda { ProfileManager.GetProgressText(false) }, [
        @READ_continue,
        {},
        {}
      ], nil)

      nil
    end
  end
end

Yast::GetProgressTextClient.new.main
