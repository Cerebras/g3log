/** ==========================================================================
* 2012 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
* with no warranties. This code is yours to share, use and modify with no
* strings attached and no restrictions or obligations.
*
* For more information see g3log/LICENSE or refer refer to http://unlicense.org
* ============================================================================*/

#include "g3log/loglevels.hpp"
#include <cassert>

#include <iostream>
#include <sstream>

namespace g3 {
   namespace internal {
      bool wasFatal(const LEVELS& level) {
         return level.value >= FATAL.value;
      }

#ifdef G3_DYNAMIC_LOGGING
      const std::map<int, LoggingLevel> g_log_level_defaults = {
	     {G3LOG_DEBUG.value,{G3LOG_DEBUG}},
         {INFO.value, {INFO}},
         {WARNING.value, {WARNING}},
         {FATAL.value, {FATAL}}
      };

      LoggingLevelContainer g_log_levels = g_log_level_defaults;

      LoggingLevelContainer::LoggingLevelContainer(const std::map<int, LoggingLevel>& init_map) :
         std::vector<LoggingLevel *>(kMaxValue, nullptr) 
      {
         for (auto& v : init_map) {
            (*this)[v.first] = new LoggingLevel(v.second);
         }
      }

      LoggingLevelContainer::~LoggingLevelContainer() {
         for (auto& v : (*this)) {
            if (v != nullptr) {
               delete v;
               v = nullptr;
            }
         }
      }
#endif
   } // internal

#ifdef G3_DYNAMIC_LOGGING
   namespace only_change_at_initialization {

      void addLogLevel(LEVELS lvl, bool enabled) {
         int value = lvl.value;
         if (value > kMaxValue) {
            std::ostringstream error;
            error << "The specified log level " << value << " is larger than the max allowed " << kMaxValue << ".\n";
            error << "The max value " << kMaxValue << " will be used.";
            std::cerr << error.str() << std::endl;
            lvl.value = kMaxValue;
            value = kMaxValue;
         }
         if (internal::g_log_levels[value] == nullptr) {
            internal::g_log_levels[value] = new LoggingLevel();
         }
         (*internal::g_log_levels[value]) = {lvl, enabled};
      }


      void addLogLevel(LEVELS level) {
         addLogLevel(level, true);
      }

      void reset() {
         g3::internal::g_log_levels = g3::internal::g_log_level_defaults;
      }
   } // only_change_at_initialization


   namespace log_levels {

      void setHighest(LEVELS enabledFrom) {
         if (internal::g_log_levels[enabledFrom.value] != nullptr) {
            for (int i=0; i<enabledFrom.value; i++) {
               if (internal::g_log_levels[i] != nullptr) {
                  disable(internal::g_log_levels[i]->level);
               }
            }
            for (int i=enabledFrom.value; i<kMaxValue; i++) {
               if (internal::g_log_levels[i] != nullptr) {
                  enable(internal::g_log_levels[i]->level);
               }
            }
         }
      }


      void set(LEVELS level, bool enabled) {
         if (internal::g_log_levels[level.value] != nullptr) {
            (*internal::g_log_levels[level.value]) = {level, enabled};
         }
      }


      void disable(LEVELS level) {
         set(level, false);
      }

      void enable(LEVELS level) {
         set(level, true);
      }


      void disableAll() {
         for (auto& p : internal::g_log_levels) {
            if (p != nullptr) {
               p->status = false;
            }
         }
      }

      void enableAll() {
         for (auto& p : internal::g_log_levels) {
            if (p != nullptr) {
               p->status = true;
            }
         }
      }


      std::string to_string(std::map<int, g3::LoggingLevel> levelsToPrint) {
         std::string levels;
         for (auto& v : levelsToPrint) {
            levels += "name: " + v.second.level.text + " level: " + std::to_string(v.first) + " status: " + std::to_string(v.second.status.value()) + "\n";
         }
         return levels;
      }

      std::string to_string() {
         return to_string(getAll());
      }


      std::map<int, g3::LoggingLevel> getAll() {
         std::map<int, g3::LoggingLevel> ret;
         for (const auto& p : internal::g_log_levels) {
            if (p != nullptr) {
               ret[p->level.value] = *p;
            }
         }
         return ret;
      }

      // status : {Absent, Enabled, Disabled};
      status getStatus(LEVELS level) {
         const auto p = internal::g_log_levels[level.value];
         if (p == nullptr) {
            return status::Absent;
         }

         return (p->status.get().load() ? status::Enabled : status::Disabled);

      }
   } // log_levels

#endif


   bool logLevel(const LEVELS& log_level) {
#ifdef G3_DYNAMIC_LOGGING
      int level = log_level.value;
      if ((level > kMaxValue) || (internal::g_log_levels[level] == nullptr)) {
         return false;
      }
      else {
         return internal::g_log_levels[level]->status.value();
      }
#endif
      return true;
   }
} // g3
