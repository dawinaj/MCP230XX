/* --- PRINTF_BYTE_TO_BINARY macro's --- */

#define PRIbit8 "%c%c%c%c%c%c%c%c"

#define PRIbit16 PRIbit8 "." PRIbit8
#define PRIbit32 PRIbit16 "," PRIbit16
#define PRIbit64 PRIbit32 ";" PRIbit32

//

#define PRItobit8(i)                  \
	(((i) & 0x80ll) ? '1' : '0'),     \
		(((i) & 0x40ll) ? '1' : '0'), \
		(((i) & 0x20ll) ? '1' : '0'), \
		(((i) & 0x10ll) ? '1' : '0'), \
		(((i) & 0x08ll) ? '1' : '0'), \
		(((i) & 0x04ll) ? '1' : '0'), \
		(((i) & 0x02ll) ? '1' : '0'), \
		(((i) & 0x01ll) ? '1' : '0')

#define PRItobit16(i) PRItobit8((i) >> 8), PRItobit8(i)
#define PRItobit32(i) PRItobit16((i) >> 16), PRItobit16(i)
#define PRItobit64(i) PRItobit32((i) >> 32), PRItobit32(i)

/* --- end macros --- */
