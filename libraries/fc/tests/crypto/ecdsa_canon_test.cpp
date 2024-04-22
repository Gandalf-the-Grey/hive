#include <fc/crypto/elliptic.hpp>
#include <fc/exception/exception.hpp>

#include <iostream>

uint8_t fc_canon[65] = {
   /*rec id */ 0x20,
   /* r */     0x12, 0x65, 0xbb, 0xa6, 0xde, 0xb1, 0xba, 0xf0, 0x79, 0x3b, 0xc5, 0x08, 0x77, 0x99, 0x27, 0x2b, 0x5d, 0x2e, 0xf6, 0xff, 0x9d, 0x72, 0x21, 0x8a, 0x68, 0x82, 0x25, 0x9d, 0x98, 0x94, 0xda, 0xd7,
   /* s */     0x66, 0x36, 0x6a, 0x28, 0xfa, 0x4d, 0xb9, 0x06, 0x66, 0x1e, 0x3b, 0xf2, 0x68, 0x2d, 0x27, 0x9b, 0xeb, 0x9d, 0x2f, 0x4c, 0xc4, 0x36, 0xee, 0xbf, 0xa3, 0x52, 0xe8, 0x4f, 0xc5, 0xd0, 0x47, 0xee
};

uint8_t bip_0062_canon[65] = {
   /*rec id */ 0x20,
   /* r */     0xe3, 0x9a, 0xff, 0x4c, 0x7a, 0xea, 0x6f, 0xe8, 0x50, 0xad, 0x9f, 0x45, 0x4a, 0xdc, 0x59, 0x61, 0x15, 0xa8, 0xfb, 0x85, 0xd7, 0xb6, 0xaa, 0x2d, 0x2a, 0x31, 0xbe, 0x84, 0x05, 0x85, 0x93, 0x5f,
   /* s */     0x56, 0x6d, 0x35, 0x9d, 0x1f, 0x57, 0x6a, 0x59, 0xb5, 0x0c, 0x4e, 0x31, 0x45, 0x04, 0x24, 0x43, 0x5a, 0x37, 0x1c, 0x35, 0x02, 0x2a, 0xab, 0x20, 0x1f, 0xcf, 0x55, 0x1b, 0xee, 0x58, 0xdd, 0x10
};

uint8_t bip_0062_canon2[65] = {
   /*rec id */ 0x20,
   /* r */     0x12, 0x65, 0xbb, 0xa6, 0xde, 0xb1, 0xba, 0xf0, 0x79, 0x3b, 0xc5, 0x08, 0x77, 0x99, 0x27, 0x2b, 0x5d, 0x2e, 0xf6, 0xff, 0x9d, 0x72, 0x21, 0x8a, 0x68, 0x82, 0x25, 0x9d, 0x98, 0x94, 0xda, 0xd7,
   /* s */     0x2f, 0x11, 0x18, 0xe5, 0xca, 0xce, 0xea, 0xce, 0x60, 0x2c, 0x8d, 0x97, 0xcb, 0x15, 0x87, 0xc1, 0x06, 0xa1, 0xe6, 0x94, 0x05, 0xe8, 0x62, 0x1e, 0xae, 0x50, 0x3c, 0xe4, 0x04, 0xad, 0x8f, 0x29
};

// -s (mod n)
uint8_t non_canon[65] = {
   /*rec id */ 0x20,
   /* r */     0x12, 0x65, 0xbb, 0xa6, 0xde, 0xb1, 0xba, 0xf0, 0x79, 0x3b, 0xc5, 0x08, 0x77, 0x99, 0x27, 0x2b, 0x5d, 0x2e, 0xf6, 0xff, 0x9d, 0x72, 0x21, 0x8a, 0x68, 0x82, 0x25, 0x9d, 0x98, 0x94, 0xda, 0xd7,
   /* s */     0xa9, 0x92, 0xca, 0x62, 0xe0, 0xa8, 0x95, 0xa6, 0x4a, 0xf3, 0xb1, 0xce, 0xba, 0xfb, 0xdb, 0xbc, 0x60, 0x77, 0xc0, 0xb1, 0xad, 0x1d, 0xf5, 0x1b, 0xa0, 0x03, 0x09, 0x70, 0xe1, 0xdd, 0x64, 0x31
};

int main( int argc, char** argv )
{
   try
   {
      fc::ecc::compact_signature fc_canon_sig;
      memcpy( fc_canon_sig.data, fc_canon, sizeof( unsigned char ) * 65 );

      fc::ecc::compact_signature bip_0062_canon_sig;
      memcpy( bip_0062_canon_sig.data, bip_0062_canon, sizeof( unsigned char ) * 65 );

      fc::ecc::compact_signature bip_0062_canon2_sig;
      memcpy( bip_0062_canon2_sig.data, bip_0062_canon2, sizeof( unsigned char ) * 65 );

      fc::ecc::compact_signature non_canon_sig;
      memcpy( non_canon_sig.data, non_canon, sizeof( unsigned char ) * 65 );

      ilog( "Testing non-canonical validation" );
      FC_ASSERT( fc::ecc::public_key::is_canonical( fc_canon_sig,          fc::ecc::canonical_signature_type::non_canonical ) );
      FC_ASSERT( fc::ecc::public_key::is_canonical( bip_0062_canon_sig,    fc::ecc::canonical_signature_type::non_canonical ) );
      FC_ASSERT( fc::ecc::public_key::is_canonical( bip_0062_canon2_sig,   fc::ecc::canonical_signature_type::non_canonical ) );
      FC_ASSERT( fc::ecc::public_key::is_canonical( non_canon_sig,         fc::ecc::canonical_signature_type::non_canonical ) );

      ilog( "Testing bip_0062 canonical validation" );
      FC_ASSERT( fc::ecc::public_key::is_canonical( fc_canon_sig,          fc::ecc::canonical_signature_type::bip_0062 ) );
      FC_ASSERT( fc::ecc::public_key::is_canonical( bip_0062_canon_sig,    fc::ecc::canonical_signature_type::bip_0062 ) );
      FC_ASSERT( fc::ecc::public_key::is_canonical( bip_0062_canon2_sig,   fc::ecc::canonical_signature_type::bip_0062 ) );
      FC_ASSERT( !fc::ecc::public_key::is_canonical( non_canon_sig,        fc::ecc::canonical_signature_type::bip_0062 ) );

      ilog( "Testing fc canonical validation" );
      FC_ASSERT(  fc::ecc::public_key::is_canonical( fc_canon_sig,         fc::ecc::canonical_signature_type::fc_canonical ) );
      FC_ASSERT( !fc::ecc::public_key::is_canonical( bip_0062_canon_sig,   fc::ecc::canonical_signature_type::fc_canonical ) );
      FC_ASSERT(  fc::ecc::public_key::is_canonical( bip_0062_canon2_sig,  fc::ecc::canonical_signature_type::fc_canonical ) );
      FC_ASSERT( !fc::ecc::public_key::is_canonical( non_canon_sig,        fc::ecc::canonical_signature_type::fc_canonical ) );
   }
   catch( fc::exception& e )
   {
      ilog( "Uncaught Exception: ${e}", ("e", e) );
      return 1;
   }

   return 0;
}
