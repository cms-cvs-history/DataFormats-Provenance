#ifndef DataFormats_Common_HashedTypes_h
#define DataFormats_Common_HashedTypes_h

// $Id: HashedTypes.h,v 1.2.2.1 2008/04/25 17:20:40 wmtan Exp $
//

/// Declaration of the enum HashedTypes, used in defining several "id"
/// classes.

namespace edm
{
  enum HashedTypes {
      ModuleDescriptionType,
      ParameterSetType,
      ProcessHistoryType,
      ProcessConfigurationType,
      EntryDescriptionType //,
      // BranchMapperType
  };		     
}

#endif // DataFormats_Common_HashedTypes_h
