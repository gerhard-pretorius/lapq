/// @file pgtype.h

#ifndef LAPQ_PGTYPE_H
#define LAPQ_PGTYPE_H


namespace lapq {
namespace pg {
//============================================================================

//////////////////////////////////////////////////////////////////////////////
/// Selected OIDs from /usr/include/postgresql/catalog/pg_type.h
enum PGOIDType
{

  PG_BOOLOID = 16,     // boolean
  PG_BYTEAOID = 17,    // variable-length string, binary values escaped
  PG_CHAROID = 18,     // single character
  PG_INT8OID = 20,     // ~18 digit integer, 8-byte storage
  PG_INT2OID = 21,     // -32 thousand to 32 thousand, 2-byte storage

  PG_INT4OID = 23,     // -2 billion to 2 billion integer, 4-byte storage
  PG_TEXTOID = 25,     // variable-length string, no limit specified
  PG_OIDOID = 26,    // object identifier(oid), maximum 4 billion

  // single-precision floating point number, 4-byte storage
  PG_FLOAT4OID = 700,

  // double-precision floating point number, 8-byte storage
  PG_FLOAT8OID = 701,

  // absolute, limited-range date and time (Unix system time)
  PG_ABSTIMEOID = 702,

  // relative, limited-range time interval (Unix delta time)
  PG_RELTIMEOID = 703,

  PG_TINTERVALOID = 704, // (abstime,abstime), time interval

  PG_UNKNOWNOID = 705,
  PG_CASHOID = 790,

  PG_MACADDROID = 829,   // XX:XX:XX:XX:XX:XX, MAC address
  PG_INETOID = 869,    // IP address/netmask, host address, netmask

  // network IP address/netmask, network address optional
  PG_CIDROID = 650,

  PG_INT4ARRAYOID = 1007,
  PG_TEXTARRAYOID = 1009,
  PG_CSTRINGARRAYOID = 1263,

  // char(length), blank-padded string, fixed storage length
  PG_BPCHAROID = 1042,

  // varchar(length), non-blank-padded string, variable storage length
  PG_VARCHAROID = 1043,

  PG_DATEOID = 1082,   // date
  PG_TIMEOID = 1083,   // time of day

  PG_TIMESTAMPOID = 1114,  // date and time"
  PG_TIMESTAMPTZOID = 1184,  // date and time with time zone
  PG_INTERVALOID = 1186,   // @ <number> <units>, time interval
  PG_TIMETZOID = 1266,     // time of day with time zone

  PG_BITOID = 1560,    // fixed-length bit string
  PG_VARBITOID = 1562,   // variable-length bit string

  // numeric(precision, decimal), arbitrary precision number
  PG_NUMERICOID = 1700
};



//============================================================================
} // namespace pg
} // namespace lapq
#endif
