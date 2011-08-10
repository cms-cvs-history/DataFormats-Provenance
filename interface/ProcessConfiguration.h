#ifndef DataFormats_Provenance_ProcessConfiguration_h
#define DataFormats_Provenance_ProcessConfiguration_h

#include <iosfwd>
#include <string>

#include "DataFormats/Provenance/interface/ParameterSetID.h"
#include "DataFormats/Provenance/interface/PassID.h"
#include "DataFormats/Provenance/interface/ReleaseVersion.h"
#include "DataFormats/Provenance/interface/ProcessConfigurationID.h"
#include "DataFormats/Provenance/interface/Transient.h"

namespace edm {
  class ProcessConfiguration {
  public:
    ProcessConfiguration();
    ProcessConfiguration(std::string const& procName,
			 ReleaseVersion const& relVersion,
			 PassID const& pass);

    ProcessConfiguration(std::string const& procName,
			 ParameterSetID const& pSetID,
			 ReleaseVersion const& relVersion,
			 PassID const& pass);
    
    std::string const& processName() const {return processName_;}
    ParameterSetID const& parameterSetID() const;
    bool isParameterSetValid() const {return parameterSetID_.isValid();}
    ReleaseVersion const& releaseVersion() const {return releaseVersion_;}
    PassID const& passID() const {return passID_;}
    ProcessConfigurationID id() const;

    void setParameterSetID(ParameterSetID const& pSetID);

    void reduce();

    struct Transients {
      Transients() : pcid_(), isCurrentProcess_(false) {}
      ProcessConfigurationID pcid_;
      bool isCurrentProcess_;
    };

  private:
    ProcessConfigurationID& pcid() const {return transients_.get().pcid_;}
    bool& isCurrentProcess() const {return transients_.get().isCurrentProcess_;}

    std::string processName_;
    ParameterSetID parameterSetID_;
    ReleaseVersion releaseVersion_; 
    PassID passID_;
    mutable Transient<Transients> transients_;
  };

  bool
  operator<(ProcessConfiguration const& a, ProcessConfiguration const& b);

  inline
  bool
  operator==(ProcessConfiguration const& a, ProcessConfiguration const& b) {
    return a.processName() == b.processName() &&
    a.parameterSetID() == b.parameterSetID() &&
    a.releaseVersion() == b.releaseVersion() &&
    a.passID() == b.passID();
  }

  inline
  bool
  operator!=(ProcessConfiguration const& a, ProcessConfiguration const& b) {
    return !(a == b);
  }

  std::ostream&
  operator<< (std::ostream& os, ProcessConfiguration const& pc);
}

#endif
