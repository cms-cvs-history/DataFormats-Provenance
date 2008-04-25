#ifndef DataFormats_CommonHashedTypes_h
#define DataFormats_CommonHashedTypes_h

// $Id: HashedTypes.h,v 1.2 2008/01/25 21:06:48 paterno Exp $
//

/// Declaration of the enum HashedTypes, used in defining several "id"
/// classes.

namespace edm
{
  enum HashedTypes 
    {
      ModuleDescriptionType,
      ParameterSetType,
      ProcessHistoryType,
      ProcessConfigurationType,
      EntryDescriptionType,
      BranchMapperType
    };		     
}

#endif // DataFormats_CommonHashedTypes_h
