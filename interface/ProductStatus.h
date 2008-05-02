#ifndef DataFormats_Provenance_ProductStatus_h
#define DataFormats_Provenance_ProductStatus_h

/*----------------------------------------------------------------------
  
ProductStatus: 

----------------------------------------------------------------------*/
/*
  ProductStatus
*/
#include <vector>

namespace edm {
  typedef unsigned char byte_t;
  typedef byte_t ProductStatus;
  namespace productstatus {
    inline ProductStatus present() {return 0x0;} // Product was made successfully
    inline ProductStatus neverCreated() {return 0x1;} // Product was not made successfully
    inline ProductStatus unknown() {return 0xfe;} // Status unknown (used for backward compatibility)
    inline bool present(ProductStatus status) {return status == present();}
    inline bool neverCreated(ProductStatus status) {return status == neverCreated();}
    inline bool unknown(ProductStatus status) {return status == unknown();}
  }
}
#endif
